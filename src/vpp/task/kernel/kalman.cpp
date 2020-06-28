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

Prediction::Prediction(const int mode, VPP::Kernel::Kalman &f) noexcept
    : VPP::Tasks::ForZones(mode), kalman(f), dt(0) {}

Error::Type Prediction::start(Scene &s, Zones &zs, float dt) noexcept {
    this->dt = dt;
    if (dt <= 0) {
        return Error::OK;
    }

    return VPP::Tasks::ForZones::start(s, zs);
}

Error::Type Prediction::wait() noexcept {
    if (dt <= 0) {
        return Error::OK;
    }

    return VPP::Tasks::ForZones::wait();
}

Error::Type Prediction::process(Scene &scn, Zone &z) noexcept {
    if (z.valid()) {

        auto &model = kalman.predictor(z);
        if (model.valid()) {
            /* Update the zone spatial contents */
            auto centre   = z.state.centre;
            auto size     = z.state.size;
            cv::Point3f c = centre;
            cv::Point3f s(size.x/2, size.y/2, 0);
            
            auto tl       = scn.view.depth.project(c-s);
            auto br       = scn.view.depth.project(c+s);
            auto geom     = br - tl;

            z.x           = tl.x;
            z.y           = tl.y;
            z.width       = geom.x;
            z.height      = geom.y;
        } else {
            z.invalidate();
        }
    }

    return Error::OK;
}

Correction::Correction(const int mode, VPP::Kernel::Kalman &f) noexcept
    : VPP::Tasks::ForZones(mode), kalman(f) {}

Error::Type Correction::process(Scene &/*s*/, Zone &z) noexcept {

    if (z.valid()) {
        auto &model = kalman.predictor(z);
        if (model.valid()) {
            model.correct(z.state);
        } else {
            z.invalidate();
        }
    }

    return Error::OK;
}

}  // namespace Kalman
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
