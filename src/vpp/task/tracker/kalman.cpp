/**
 *
 * @file      vpp/task/tracker/kalman.cpp
 *
 * @brief     These are various tasks to efficiently use Kalman trackers in a
 *            scene
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

#include "vpp/task.hpp"
#include "vpp/task/tracker/kalman.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace Task {
namespace Tracker {
namespace Kalman {

Prediction::Prediction(const int mode, VPP::Tracker::Kalman::Engine &e) noexcept
    : Parent(mode), kalman(e), dt(0) {}

Error::Type 
    Prediction::start(Scene &s, float dt,
                      VPP::Tracker::Kalman::Contexts &ctx) noexcept {
    this->dt = dt;
    if (dt <= 0) {
        return Error::OK;
    }
    return Parent::start(ctx, s);
}

Error::Type Prediction::start(Scene &s, float dt) noexcept {
    contexts = std::move(kalman.contexts(kalman.history_contexts));
    return start(s, dt, contexts);
}

Error::Type Prediction::wait() noexcept {
    if (dt <= 0) {
        return Error::OK;
    }

    return Parent::wait();
}

Error::Type Prediction::process(VPP::Tracker::Kalman::Context &c, 
                                Scene &scn) noexcept {
    c.predict(scn.view, dt);
    return Error::OK;
}

Correction::Correction(const int mode, VPP::Tracker::Kalman::Engine &e) noexcept
    : Parent(mode), kalman(e) {}

Error::Type 
    Correction::start(Scene &s, VPP::Tracker::Kalman::Contexts &ctx) noexcept {
    return Parent::start(ctx, s);
}

Error::Type Correction::start(Scene &s) noexcept {
    contexts = std::move(kalman.contexts(kalman.history_contexts));
    return start(s, contexts);
}

Error::Type Correction::process(VPP::Tracker::Kalman::Context &c, 
                                Scene &/*s*/) noexcept {
    c.correct();
    return Error::OK;
}

}  // namespace Kalman
}  // namespace Tracker
}  // namespace Task
}  // namespace VPP
