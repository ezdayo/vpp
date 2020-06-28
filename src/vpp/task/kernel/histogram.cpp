/**
 *
 * @file      vpp/task/kernel/histogram.cpp
 *
 * @brief     These are various tasks to efficiently use Histogram kernels in a
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

#include <opencv2/video/tracking.hpp>

#include "vpp/task.hpp"
#include "vpp/task/kernel/histogram.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace Task {
namespace Kernel {
namespace Histogram {

CamShift::CamShift(const int mode, VPP::Kernel::Histogram &h) noexcept
    : VPP::Tasks::ForZones(mode), histogram(h), term() {}

Error::Type CamShift::start(Scene &s, Zones &zs) noexcept {
    /* Cache the right mode for the view */
    s.view.cache(histogram.mode());
    return VPP::Tasks::ForZones::start(s, zs);
}

Error::Type CamShift::process(Scene &scn, Zone &z) noexcept {
    if (z.valid()) {
        auto &data = histogram.data(scn.view, z);
        auto dest  = std::move(data.back_project(scn.view));
        cv::Rect estimated(z);
        cv::CamShift(dest, estimated, term);
        /* Find a way to track new estimated positions [WORK ON COPIES ?] or 
         * put it back in the provided zone ? Maybe a zone update is handy to
         * update the zone with the new setup ? */
    }

    return Error::OK;
}

}  // namespace Histogram
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
