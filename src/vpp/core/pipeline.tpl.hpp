/**
 *
 * @file      vpp/core/pipeline.tpl.hpp
 *
 * @brief     This is the VPP pipeline implementation
 *
 * @details   This is the class for describing a pipeline
 *
 *            This file is part of the VPP framework (see link).
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#pragma once

#include <forward_list>

#include "vpp/log.hpp"
#include "vpp/core/pipeline.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> Pipeline<Z...>::Pipeline() noexcept 
    : Customisation::Entity("Pipeline"), finished(),
      stages(), run(false), retry(false), halt(false), zombie(false),
      resume(), suspend(), thread() {
    /* Define the running parameter */
    running.denominate("running");
    running.describe("Is the pipeline running ?");
    running = false;
    running.use(Customisation::Translator::BoolFormat::NO_YES);
    running.trigger([this](const bool &yes) {
                    return this->onRunningUpdate(yes); });
    expose(running).characterise(Customisation::Trait::SETTABLE);

    /* Define the frozen parameter */
    frozen.denominate("frozen");
    frozen.describe("Is the pipeline frozen ?");
    frozen = false;
    frozen.use(Customisation::Translator::BoolFormat::NO_YES);
    frozen.trigger([this](const bool &yes) {
                   return this->onFrozenUpdate(yes); });
    expose(frozen).characterise(Customisation::Trait::SETTABLE);
}
 
template <typename ...Z> Pipeline<Z...>::~Pipeline() noexcept {
    /* Cleanly exit the thread to prevent program termination */
    terminate();
}

template <typename ...Z> void Pipeline<Z...>::terminate() noexcept {
    /* Cleanly exit the thread to prevent program termination */
    unfreeze();
    stop();
}

template <typename ...Z>
Pipeline<Z...> &Pipeline<Z...>::operator>>(Stage &stage)
    noexcept {
    
    {
        /* Inside a lock_guard scoped block */
        std::lock_guard<std::mutex> lock(suspend);
 
        /* This shall never happen */
        ASSERT(((!run) && (!thread.joinable())),
               "%s[%s]:operator >>() called whilst thread is running!", 
                 value_to_string().c_str(), name().c_str());

        stages.emplace_back(stage); 
    }

    ASSERT((!stage.name().empty()),
            "%s[%s]:operator >>(Stage &) cannot bound an unnamed stage!",
            value_to_string().c_str(), name().c_str());
    
    expose(stage);
 
    return *this;
}

template <typename ...Z> void Pipeline<Z...>::start() noexcept {
    running = true;
}

template <typename ...Z> void Pipeline<Z...>::stop() noexcept {
    running = false;
}

template <typename ...Z> void Pipeline<Z...>::freeze() noexcept {
    frozen = true;
}

template <typename ...Z> void Pipeline<Z...>::unfreeze() noexcept {
    frozen = false;
}

template <typename ...Z>
    void Pipeline<Z...>::work(Scene* &s, Z*&... z) noexcept {
    bool notify = false;

    while (true) {
        auto error = process(s, z...);

        /* Error handling: exiting with broken empty scene */
        if (error) {

            /* Only broadcast actual errors and not retry or not ready status */
            if (error < 0) {
                LOGE("%s[%s]:process() error %d!", 
                      value_to_string().c_str(), name().c_str(), error);
                broadcast.signal(*s, *z..., error);
            }
        }

        { 
            /* Lock and wait safely inside scoped block */
            std::unique_lock<std::mutex> lock(suspend);
              
            /* If there is a not ready error and a retry pending then give it
             * another try ! */
            bool do_retry = (error == Error::RETRY) || 
                            ( (error == Error::NOT_READY) && (retry) );

            bool do_exit = ( (!run) || (error < 0) ||
                             ( (error == Error::NOT_READY) && (!retry) ));

            retry = false;

            /* If no longer running or if an error happened, then exit */
            if (do_exit) {
                /* Flushing what's inside and beyond the pipeline */
                flush();
                run    = false;
                halt   = false;
                zombie = true;
                resume.notify_all();
                return;
            }

            /* Notify only if not halted, and wait for halt clearance
             * otherwise */
            notify = (!halt) && (!do_retry); 

            if (halt) {
                resume.wait(lock, [this] { return !this->halt; } );
            }
        }

        if (notify) {
            broadcast.signal(*s, *z..., error);
            if (finished != nullptr) {
                finished(*s, *z...);
            }
        } 
    }
}

template <typename ...Z> Error::Type
    Pipeline<Z...>::process(Scene* &s, Z*&... z) noexcept {

    ASSERT((!stages.empty()), "%s[%s]::process() is empty!",
            value_to_string().c_str(), name().c_str());
    
    if (stages.empty()) {
        return Error::NOT_EXISTING;
    }

    prepare(s, z...);
    for (auto &stage : stages) { 
         auto error = stage.get().prepare(s, z...);
         /* Stop at the the first encountered error */
         if (error != Error::NONE) {
             return error;
         }
         
         error = stage.get().process(*s, *z...);
         if (error != Error::NONE) {
             return error;
         }
    }

    return Error::NONE;
}
        
template <typename ...Z>
Customisation::Error Pipeline<Z...>::onRunningUpdate(bool yes) noexcept {
    
    {  
        /* Only allow pipeline running when the component is locked! */ 
        if (yes && ((traits() & Customisation::Trait::LOCKED) != 
                    (Customisation::Trait::LOCKED))) {
                return Customisation::Error::NONE;            
        }

        /* Lock and wait safely inside scoped block */
        std::unique_lock<std::mutex> lock(suspend);

        /* A joinable thread is a running thread */
        bool joinable = thread.joinable();

        /* In any case, the pipeline is no longer halted, since we modify its
         * running status: requesting a running thread cannot be halted nor can
         * be a stopped one! */
        if (halt) {
            halt = false;
            resume.notify_all();
        }

        /* If the pipeline is in a coherent state, then leave ASAP */
        if ( (run == yes) && (run == joinable) ) {
            /* If requesting to start again whilst running, this is likely a
             * retry man... */
            if (run) { 
                retry = true; 
            } 
            return Customisation::Error::NONE;
        }

        /* If the thread is a zombie thread, then join it */
        if (zombie) {
            thread.join();
            zombie = false;
            joinable = false;
        }

        /* From now on, there cannot be any zombie thread */
        if (yes != joinable) {
            run = yes;
            if (yes) {
                thread = std::thread([this] { return this->launch(); } );
                return Customisation::Error::NONE;
            } 
            else {
                /* If no longer running then flush the pipeline content */
            }
        }

        /* Wait for any resume event, including a zombie notification. Use a
         * 10 ms watchdog to mitigate race conditions with another thread that
         * is changing the status of this pipeline */
        resume.wait_for(lock, std::chrono::milliseconds(10));
    }

    /* Tail recursion only, BUT keep it outside the unique_lock scope block to
     * prevent deadlocks... */
    return onRunningUpdate(yes);
}

template <typename ...Z>
Customisation::Error Pipeline<Z...>::onFrozenUpdate(bool yes) noexcept {    
    /* Inside a lock_guard scoped block, as we need to access thread status
     * variables */
    std::lock_guard<std::mutex> lock(suspend);

    /* If the pipeline is not running or if it is in the right state, then we
     * are done */
    if (halt == (yes && run)) {
        return Customisation::Error::NONE;
    } 
    
    /* Update the halt status and notify the pipeline, once out of the mutex */
    halt = yes & run;
    resume.notify_all();
    
    return Customisation::Error::NONE;
}

}  // namespace Core
}  // namespace VPP
