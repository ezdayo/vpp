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

Initialiser::Initialiser(const int mode, 
                         VPP::Kernel::Histogram::Engine &e) noexcept
    : Parent(mode), histogram(e) {}

Error::Type Initialiser::start(Scene &s, Zones &zs) noexcept {
    /* Cache the right mode for the view and prepare the engine for the new
     * zones */
    s.view.cache(histogram.mode());
    histogram.prepare(zs);
    /* Only consider the new (original) contexts */
    auto contexts = std::move(histogram.contexts(histogram.original_contexts));
    return Parent::start(contexts, s);
}

Error::Type Initialiser::process(VPP::Kernel::Context &c, Scene &scn) noexcept {
    
    ASSERT(c.original != nullptr,
           "Task::Kernel::Histogram::Initialiser::process(): Cannot initialise "
           "old contexts");

    histogram.context(c).initialise(scn.view);

    return Error::OK;
}


CamShift::CamShift(const int mode, VPP::Kernel::Histogram::Engine &e) noexcept
    : Parent(mode), histogram(e), term() {}

Error::Type CamShift::start(Scene &s) noexcept {
    /* Cache the right mode for the view */
    s.view.cache(histogram.mode());
    auto contexts = std::move(histogram.contexts(histogram.history_contexts));
    return Parent::start(contexts, s);
}

Error::Type CamShift::process(VPP::Kernel::Context &c, Scene &scn) noexcept {
    if (c.zone().valid()) {
        auto dest  = std::move(histogram.context(c).back_project(scn.view));
        cv::Rect estimated(c.zone());
        cv::CamShift(dest, estimated, term);
        auto &z = c.stack(Zone(estimated));
        z.deproject(scn.view);
    }

    return Error::OK;
}

}  // namespace Histogram
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
