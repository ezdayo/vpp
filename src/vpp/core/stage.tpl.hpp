/**
 *
 * @file      vpp/core/stage.tpl.hpp
 *
 * @brief     This is the VPP pipeline stage implementation
 *
 * @details   This is the class for describing a pipeline stage
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

#include "vpp/log.hpp"
#include "vpp/core/stage.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> Stage<Z...>::Stage(bool update) noexcept 
    : Customisation::Entity("Stage"), filter(), broadcast(), skipped(false),
      runpdatable(update), engines(), pEngine(nullptr), suspend() {

    /* Define the bypassed parameter */
    bypassed.denominate("bypassed")
            .describe("Is the pipeline stage bypassed ?")
            .characterise(Customisation::Trait::SETTABLE);

    bypassed = false;
    bypassed.use(Customisation::Translator::BoolFormat::NO_YES);
    bypassed.trigger([this](const bool &yes) { 
                     return onBypassedUpdate(yes); });
    expose(bypassed);
            
    /* Define the enabled parameter */
    disabled.denominate("disabled")
            .describe("Is the pipeline stage disabled ?")
            .characterise(Customisation::Trait::CONFIGURABLE);

    disabled = false;
    disabled.use(Customisation::Translator::BoolFormat::NO_YES);
    disabled.trigger([this](const bool &yes) { 
                     return onDisabledUpdate(yes); });
    expose(disabled);
            
    /* Define the uses parameter */
    engine.denominate("uses")
          .describe("Name of the current engine")
          .characterise(Customisation::Trait::SETTABLE);
    engine.trigger([this](const std::string &eng) { 
                   return onEngineUpdate(eng); });
    expose(engine);
}

template <typename ...Z> void Stage<Z...>::bypass(bool yes) noexcept {
    bypassed = yes;
}

template <typename ...Z> void Stage<Z...>::disable(bool yes) noexcept {
    disabled = yes;
}

template <typename ...Z> 
Customisation::Error Stage<Z...>::use(const std::string id, 
                                      Engine &engine) noexcept {
   
    ASSERT((!id.empty()) && (engines.find(id) == engines.end()),
           "%s[%s]::use(\"%s\", Engine &) called with an invalid setup %d!", 
           value_to_string().c_str(), name().c_str(), id.c_str(), 
           (engines.find(id) != engines.end()));
    
    {
        /* Inside a lock_guard scoped block */
        std::lock_guard<std::mutex> lock(suspend);

        engines.emplace(id, engine);

        /* Allow the engine parameter to use this engine object */
        this->engine.allow(id);
    }

    /* Name and bind this engine object to this stage */
    expose(engine.denominate(std::move(id)));

    return use(engine.name());
}

template <typename ...Z> 
Customisation::Error Stage<Z...>::use(const std::string &id) noexcept {
    return engine.set(id);
}

template <typename ...Z>
    Error::Type Stage<Z...>::prepare(Scene*& s, Z*&...z) noexcept {

    Engine *the_engine = nullptr;
    
    {
        /* Inside a lock_guard scoped block */
        std::lock_guard<std::mutex> lock(suspend);

        ASSERT((pEngine != nullptr),
                "%s[%s]::prepare() has no valid engine set!",
                value_to_string().c_str(), name().c_str());
         the_engine = pEngine;
    }

    if  (the_engine != nullptr) {
        return the_engine->prepare(s, z...);
    } else {
        return Error::NOT_EXISTING;
    }

}

template <typename ...Z>
    Error::Type Stage<Z...>::process(Scene &s, Z&...z) noexcept {

    Error::Type error  = Error::NONE;
    Engine *the_engine = nullptr;
    
    {
        /* Inside a lock_guard scoped block */
        std::lock_guard<std::mutex> lock(suspend);

        if (!skipped) {
            ASSERT((pEngine != nullptr),
                    "%s[%s]::process() has no valid engine set!",
                    value_to_string().c_str(), name().c_str());
            the_engine = pEngine;
        }
    }

    if  ( (the_engine != nullptr) && 
          ( (filter == nullptr) || (filter(s, z...)) ) ) {
        error = the_engine->process(s, z...);
    }

    broadcast.signal(s, z..., error);
  
    return error;
}

template <typename ...Z> 
Customisation::Error Stage<Z...>::onBypassedUpdate(const bool &yes) noexcept {
        
    /* Inside a lock_guard scoped block */
    std::lock_guard<std::mutex> lock(suspend);

    /* A disabled stage is always skipped! */
    skipped = yes || disabled;

    return Customisation::Error::NONE;
}

template <typename ...Z> 
Customisation::Error Stage<Z...>::onDisabledUpdate(const bool &yes) noexcept {
    
    Customisation::Entity::disable(yes);

    /* Bypass a disabled stage */
    if (yes) {
        bypassed = yes;
    }

    return Customisation::Error::NONE;
}

template <typename ...Z>
Customisation::Error Stage<Z...>::onEngineUpdate(const std::string &id)
    noexcept {

    Customisation::Error error = Customisation::Error::NONE;

    if ( (locked()) && (!runpdatable) ) {
        return Customisation::Error::INVALID_REQUEST;
    }

    /* Inside a lock_guard scoped block */
    std::lock_guard<std::mutex> lock(suspend);
    
    /* If alredy using the right engine, then we are good to go! */
    if ((pEngine == nullptr) || (pEngine->name() != id)) {
   
        if ( (!runpdatable) && (pEngine != nullptr) ) {
            pEngine->disable();
        }

        /* Otherwise, search for the right engine */
        auto found = engines.find(id);
        if  (found != engines.end()) {
            pEngine = &found->second.get();
            pEngine->enable();
        } else {
            error = Customisation::Error::NOT_EXISTING;
        }
    }

    return error;
}

}  // namespace Core
}  // namespace VPP
