/**
 *
 * @file      vpp/task/kernel/kalman.cpp
 *
 * @brief     These are various tasks to efficiently use Kalman kernels in a
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
#include "vpp/task/kernel/kalman.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace Task {
namespace Kernel {
namespace Kalman {

Prediction::Prediction(const int mode, VPP::Kernel::Kalman::Engine &e) noexcept
    : Parent(mode), kalman(e), dt(0) {}

Error::Type Prediction::start(Scene &s, float dt) noexcept {
    this->dt = dt;
    if (dt <= 0) {
        return Error::OK;
    }

    auto contexts = std::move(kalman.contexts(kalman.history_contexts));
    return Parent::start(contexts, s);
}

Error::Type Prediction::wait() noexcept {
    if (dt <= 0) {
        return Error::OK;
    }

    return Parent::wait();
}

Error::Type Prediction::process(VPP::Kernel::Context &c, Scene &scn) noexcept {
    kalman.context(c).predict(scn.view, dt);
    return Error::OK;
}

Correction::Correction(const int mode, VPP::Kernel::Kalman::Engine &e) noexcept
    : Parent(mode), kalman(e) {}

Error::Type Correction::start(Scene &s) noexcept {
    auto contexts = std::move(kalman.contexts(kalman.history_contexts));
    return Parent::start(contexts, s);
}

Error::Type Correction::process(VPP::Kernel::Context &c, 
                                Scene &/*s*/) noexcept {
    kalman.context(c).correct();
    return Error::OK;
}

}  // namespace Kalman
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
