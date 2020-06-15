/**
 *
 * @file      vpp/core/task.hpp
 *
 * @brief     This is the VPP task core description file
 *
 * @details   This is the definition of a task. A task is a standalone unit of
 *            processing that can be run synchronously or in parallel, typically
 *            in the context of an engine.
 *            Each and every one pipeline task shall be derived from this class.
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

#include <future>
#include <mutex>
#include <string>

#include "customisation/entity.hpp"
#include "vpp/error.hpp"
#include "vpp/scene.hpp"
#include "vpp/util/task.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> class Task : public Parametrisable, Util::Task {
    public:

        using Util::Task::Mode;
        using Util::Task::wait;

        /**
         * Creating a new task
         *
         * @mode: the mode of the task and the maximal number of active
         *        asynchronous threads
         */
        explicit Task(const int mode) noexcept;

        /** Tasks cannot be copied nor moved */
        Task(const Task& other) = delete;
        Task(Task&& other) = delete;
        Task& operator=(const Task& other) = delete;
        Task& operator=(Task&& other) = delete;
        virtual ~Task() noexcept = default;

        /** Start processing scene and its environment */
        Error::Type start(Scene &s, Z&... z) noexcept;

      protected:
        /** Do the actual scene and environment processing */
        virtual Error::Type process(Scene &s, Z&... z) noexcept;
};

template <typename O, typename ...I> 
    class Tasks : public Parametrisable, Util::Task {
    public:

        using Util::Task::Mode;
        using Util::Task::wait;

        /**
         * Creating a new task
         *
         * @mode: the mode of the task and the maximal number of active
         *        asynchronous threads
         */
        explicit Tasks(const int mode) noexcept;

        /** Tasks cannot be copied nor moved */
        Tasks(const Tasks& other) = delete;
        Tasks(Tasks&& other) = delete;
        Tasks& operator=(const Tasks& other) = delete;
        Tasks& operator=(Tasks&& other) = delete;
        virtual ~Tasks() noexcept = default;

        /** Start processing scene and its environment */
        Error::Type start(Scene &s, I&... i) noexcept;

    protected:
        /** Iterator to do things in parallel */
        virtual Error::Type next(Scene &s, O& o, I&... i) noexcept;

        /** Do the actual scene and environment processing */
        virtual Error::Type process(Scene &s, O& o) noexcept;

        /** Mutex for synchronising access between tasks */
        std::mutex synchro;
};

}  // namespace Core
}  // namespace VPP
