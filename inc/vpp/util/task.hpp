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

#include <forward_list>
#include <future>
#include <string>

namespace Util {

class Task {
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
        explicit Task(const int mode) noexcept;

        /** Tasks can be copied and moved */
        Task(const Task& other) = default;
        Task(Task&& other) = default;
        Task& operator=(const Task& other) = default;
        Task& operator=(Task&& other) = default;
        virtual ~Task() noexcept = default;

        /** Start processing a task */
        int start(Work work) noexcept;

        /** Wait for the processing to be done */
        int wait() noexcept;

      private:
        int                                  _mode;    /// Task mode
        int                                  _error;   /// Task error
        std::forward_list<std::future<int> > _status;  /// Status when task ends
};

}  // namespace Util
