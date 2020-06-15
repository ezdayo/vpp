/**
 *
 * @file      vpp/core/task.tpl.hpp
 *
 * @brief     This is the VPP task implementation
 *
 * @details   This is the base class of all pipeline tasks.
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

#include "vpp/log.hpp"
#include "vpp/core/task.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> Task<Z...>::Task(const int mode) noexcept
    : Customisation::Entity("Task"), Util::Task(mode) {
    
    ASSERT(((mode == Mode::Async) || (mode == Mode::Sync) || 
            (mode == Mode::Lazy)),
            "%s[%s]::Task(): Invalid mode provided %d!",
            value_to_string().c_str(), name().c_str(), mode);
}

template <typename ...Z> Error::Type
    Task<Z...>::start(Scene &s, Z&... z) noexcept {
    return Util::Task::start([this, &s, &z...] { return process(s, z...); });
}

template <typename ...Z>
    Error::Type Task<Z...>::process(Scene &/*s*/, Z&... /*z*/) noexcept {
    LOGE("%s[%s]::process(Scene &...) is undefined!",
          value_to_string().c_str(), name().c_str());

    return Error::UNDEFINED;
}

template <typename O, typename ...I> 
    Tasks<O, I...>::Tasks(const int mode) noexcept
        : Customisation::Entity("Task"), Util::Task(mode) {}

template <typename O, typename ...I> Error::Type
    Tasks<O, I...>::start(Scene &s, I&... i) noexcept {
    return Util::Task::start([this, &s, &i...] 
            { 
                int error = INT_MAX;
                while (true) {
                    O o;
                    {
                        std::lock_guard<std::mutex> lock(synchro);
                        auto e = next(s, o, i...);
                        if (e != Error::OK) {
                            if (e != Error::NOT_EXISTING) {
                                return e;
                            }
                            return error;
                        }
                    }
                    auto e = process(s, o);
                    if (e < Error::OK) {
                        return e;
                    }
                    if (e < error) {
                        error = e;
                    }
                }
            });
}

template <typename O, typename ...I>
    Error::Type 
        Tasks<O, I...>::next(Scene &/*s*/, O& /*o*/, I&.../*i*/) noexcept {
    LOGE("%s[%s]::next(Scene&, O&, I&...) is undefined!",
          value_to_string().c_str(), name().c_str());

    return Error::UNDEFINED;
}

template <typename O, typename ...I>
    Error::Type Tasks<O, I...>::process(Scene &/*s*/, O& /*o*/) noexcept {
    LOGE("%s[%s]::process(Scene , O&) is undefined!",
          value_to_string().c_str(), name().c_str());

    return Error::UNDEFINED;
}

}  // namespace Core
}  // namespace VPP
