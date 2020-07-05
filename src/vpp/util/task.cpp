/**
 *
 * @file      vpp/util/task.cpp
 *
 * @brief     This is a task wrapper implementation
 *
 * @details   This is the base class of all tasks.
 *            Each and every one task shall be derived from this class.
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

#include <algorithm>
#include <climits>

#include "vpp/util/task.hpp"

namespace Util {
namespace Task {

Core::Core(const int mode) noexcept : _error(0) {
    int cnt = abs(mode);
    
    for (int i=0; i < std::min(16, cnt); ++i) {
        _status.emplace_front();
    }

    if (cnt > 0) {
        _mode = mode / cnt;
    } else {
        _mode = 0;
    }
}

int Core::start(Work work) noexcept {
    if (_mode == Mode::Sync) {
        _error = work();
        return 0;
    }

    std::launch kind;
    if (_mode == Mode::Async) {
        kind = std::launch::async;
    } else {
        kind = std::launch::deferred;
    }
 
    for (auto &s : _status) {
        s = std::async(kind, work);
    }

  return 0;
}

int Core::wait() noexcept {
    if (_mode != Mode::Sync) {

        _error = INT_MAX;
        for (auto &s : _status) {
            int e = s.get();
            if (e < _error) {
                _error = e;
            }
        }
    }

    return _error;
}

}  // namespace Task
}  // namespace Util
