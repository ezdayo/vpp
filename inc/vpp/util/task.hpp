/**
 *
 * @file      vpp/util/task.hpp
 *
 * @brief     This is a task wrapper description file
 *
 * @details   This is the definition of a generic task. A task is a standalone
 *            unit of execution that can be run synchronously or in parallel.
 *            This is a simple wrapper to the std::task class.
 *            Tasks can be nested into other tasks, so that one task can throw
 *            multiple sub-tasks in parallel.
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

#include <cassert>
#include <forward_list>
#include <future>
#include <string>

#include "vpp/util/templates.hpp"

namespace Util {
namespace Task {

class Core {
    public:

        /** Operating mode of task (deferred or asynchronous launch) */
        class Mode {
            public:
                /** Lazy evaluation */
                static constexpr int Lazy  = -1;
                /** Immediate (synchronous) launch */
                static constexpr int Sync  =  0;
                /** Asynchronous launch */
                static constexpr int Async =  1;
            };

        using Work = std::function<int(void) noexcept>;

        /**
         * Creating a new task
         *
         * @mode: the mode of the task is a mix between a type of operation, 
         *        i.e. Mode::Async for asynchronous tasks, Mode::Lazy for lazy
         *        deferred tasks or Mode::Sync for synchronous task to which a
         *        multiplier can me added to define the maximum number of
         *        asynchronous threads
         */
        explicit Core(const int mode) noexcept;
        ~Core() noexcept = default;

        /** Tasks cannot be copied nor moved */
        Core(const Core& other) = delete;
        Core(Core&& other) = delete;
        Core& operator=(const Core& other) = delete;
        Core& operator=(Core&& other) = delete;

        /** Start processing a task */
        int start(Work work) noexcept;

        /** Wait for the processing to be done */
        int wait() noexcept;

      private:
        int                                  _mode;    /// Task mode
        int                                  _error;   /// Task error
        std::forward_list<std::future<int> > _status;  /// Status when task ends
};

/* Using the curiously recurring template pattern (CRTP) for performance
 * T is the final task class, E is the optional environment parameter 
 * references. This class instantiate a single task for performing actions in 
 * background or so... */
template <typename T, typename ...E> class Single : public Core {
    public:
        inline explicit Single(const int mode) noexcept : Core(mode) {
            /* Prevent using more than one task for a single task */
#ifndef NDEBUG 
            assert(((mode == Mode::Async) || (mode == Mode::Sync) || 
                   (mode == Mode::Lazy)));
#endif /*!NDEBUG*/ 
        }
        inline ~Single() noexcept = default;

        /** Start processing the optional environment */
        inline int start(E&... e) noexcept {
            /* This is where the trick is: Do not force passing by reference, 
             * after the lambda function parameters so that non-referenced 
             * objects are instantiated and have a storage space in the dispatch
             * function parameters, whereas referenced one keep the intial
             * reference. It is not possible to do the same in the lambda
             * function call, as the lambda does not have an explicit signature.
             */
            return Core::start(std::move([this, &e...] { 
                return dispatch(e...); }));
        }

    protected:
        inline int dispatch(E... e) noexcept {
            return static_cast<T*>(this)->process(e...);
        }

        /** Do the actual processing */
        inline int process(E&... /*e*/) noexcept {
            /* process(): Process shall be redefined in child classes! */
#ifndef NDEBUG 
            assert(false);
#endif /*!NDEBUG*/ 
            return -1;
        }
};

}  // namespace Task

namespace Tasks {

/* Using the curiously recurring template pattern (CRTP) for performance
 * T is the final task class, E is the optional environment parameters, I is the
 * This class instantiate multiple tasks for performing what is in process
 * actions in background unless deferred... */
template <typename T, typename ...E> class Core : public Util::Task::Core {
    public:
        using typename Util::Task::Core::Mode;

        inline explicit Core(const int mode) noexcept
            : Util::Task::Core(mode), synchro() {}
        inline ~Core() noexcept = default;

        /** Start processing the environment */
        inline int start(E&... e) noexcept {
           /* This is where the trick is: Do not force passing by reference, 
             * after the lambda function parameters so that non-referenced 
             * objects are instantiated and have a storage space in the dispatch
             * function parameters, whereas referenced one keep the intial
             * reference. It is not possible to do the same in the lambda
             * function call, as the lambda does not have an explicit signature.
             */
            return Util::Task::Core::start([this, &e...] { 
                return dispatch(e...); });
        }

    protected:
        inline int dispatch(E... e) noexcept {
            std::unique_lock<std::mutex> access(synchro, std::defer_lock);
            int error  = 0;
            bool again = true;
            while (again) {
                access.lock();
                again = static_cast<T *>(this)->next(e...);
                access.unlock();
                if (again) {
                    error = static_cast<T *>(this)->process(e...);
                    if (error < 0) {
                        again = false;
                    }
                }
            }
            return error;
        }

        /** Iterator to do things in parallel */
        inline bool next(E&... /*e*/) noexcept {
            /* next(): Next environment computation shall be redefined in child
             * classes! */
#ifndef NDEBUG 
            assert(false);
#endif /*!NDEBUG*/ 
            return false;
        }
        
        /** Where to do the actual environment processing */
        inline int process(E&... /*e*/) noexcept {
            /* process(): Process shall be redefined in child classes! */
#ifndef NDEBUG 
            assert(false);
#endif /*!NDEBUG*/ 
            return -1;
        }

        /** Mutex for synchronising access between tasks */
        std::mutex            synchro;
};

/* Tasks list are only relevant when the L type is a container! 
 * This class manages containers of reference_wrappers as if they were 
 * standard (plain) containers */
template <typename T, typename L, typename ...E> 
    class List : public Core<T, containee_object_t<L>*, E...> {
    public:
        using O = containee_object_t<L>;
        using Parent = Core<T, O*, E...>;
        using typename Parent::Mode;

        inline explicit List(const int mode) noexcept 
            : Parent(mode), synchro(), list(), it() {}
        inline ~List() noexcept = default;

        /** Start processing the environment */
        inline int start(L l, E&... e) noexcept {
            list = std::move(storable_wrapper_t<L>(l));
            if (list.empty()) {
                return 0;
            }
            O *o = nullptr;
            it = list.begin();
            return Parent::start(o, e...);
        }
        
        /** Wait for the processing to be done */
        inline int wait() noexcept {
            if (list.empty()) {
                return 0;
            }
            return Parent::wait();
        }

    protected:
        /** Iterator to do things in parallel */
        inline bool next(O* &o, E&... /*e*/) noexcept {
            if (it == list.end()) {
                return false;
            }
            /* The cast to O& does nothing with standard containee objects, but
             * it does de-reference a reference-wrapped object */
            o = &(static_cast<O&>(*it));
            ++it;
            return true;
        }

        /* This is a wrapper for the actual processing function */
        inline int process(O* o, E&... e) noexcept {
            return static_cast<T*>(this)->process(*o, e...);
        }

        /** Where to do the actual environment processing */
        inline int process(O& /*o*/, E&... /*e*/) noexcept {
            /* process(): Process shall be redefined in child classes! */
#ifndef NDEBUG 
            assert(false);
#endif /*!NDEBUG*/ 
            return -1;
        }

        /** Mutex for synchronising access between tasks */
        std::mutex            synchro;

        /** Iterator on the list of tasks */
        storable_wrapper_t<L> list;
        Util::iterator_t<L>   it;
};

/* Tasks lists are only relevant when the X and Y types are containers! 
 * This class manages containers of reference_wrappers as if they were 
 * standard (plain) containers */
template <typename T, typename X, typename Y, typename ...E> 
    class Lists : public Core<T, containee_object_t<X>*, 
                              containee_object_t<Y>*, E...> {
    public:
        using XO = containee_object_t<X>;
        using YO = containee_object_t<Y>;
        using Parent = Core<T, XO*, YO*, E...>;
        using typename Parent::Mode;

        inline explicit Lists(const int mode) noexcept 
            : Parent(mode), synchro(), xlist(), ylist(), xit(), yit() {}
        inline ~Lists() noexcept = default;

        /** Start processing the environment */
        inline int start(X x, Y y, E&... e) noexcept {
            xlist = std::move(storable_wrapper_t<X>(x));
            ylist = std::move(storable_wrapper_t<Y>(y));
            if (xlist.empty() || ylist.empty()) {
                return 0;
            }
            XO *xo = nullptr;
            YO *yo = nullptr;
            xit = xlist.begin(); 
            yit = ylist.begin(); 
            return Parent::start(xo, yo, e...);
        }

        /** Wait for the processing to be done */
        inline int wait() noexcept {
            if (xlist.empty() || ylist.empty()) {
                return 0;
            }
            return Parent::wait();
        }

    protected:
        /** Iterator to do things in parallel */
        inline bool next(XO* &xo, YO* &yo, E&... /*e*/) noexcept {
            if ((xit == xlist.end()) && (yit == ylist.end())) {
                return false;
            }

            /* The cast to O& does nothing with standard containee objects, but
             * it does de-reference a reference-wrapped object */
            if (yit != ylist.end()) {
                xo = &(static_cast<XO&>(*xit));
                yo = &(static_cast<YO&>(*yit));
                ++yit;
                return true;
            }
            yit = ylist.begin();
            ++xit;
            if (xit != xlist.end()) {
                xo = &(static_cast<XO&>(*xit));
                yo = &(static_cast<YO&>(*yit));
                return true;
            }
            return false;
        }

        /** This is a wrapper for the actual processing function */
        inline int process(XO* xo, YO* yo, E&... e) noexcept {
            return static_cast<T*>(this)->process(*xo, *yo, e...);
        }

        /** Where to do the actual environment processing */
        inline int process(XO& /*xo*/, YO& /*yo*/, E&... /*e*/) noexcept {
            /* process(): Process shall be redefined in child classes! */
#ifndef NDEBUG 
            assert(false);
#endif /*!NDEBUG*/ 
            return -1;
        }

        /** Mutex for synchronising access between tasks */
        std::mutex            synchro;

        /** Iterators on the lists of tasks */
        storable_wrapper_t<X> xlist;
        storable_wrapper_t<Y> ylist;
        Util::iterator_t<X>   xit;
        Util::iterator_t<Y>   yit;
};

}  // namespace Tasks
}  // namespace Util
