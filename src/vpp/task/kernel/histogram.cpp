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
    : Parent(mode), contexts(), histogram(e) {}

Error::Type Initialiser::start(Scene &s, Zones &zs, 
                               VPP::Kernel::Histogram::Contexts &ctx)
    noexcept {
    /* Cache the right mode for the view and prepare the engine for the new
     * zones */
    s.view.cache(histogram.mode());
    histogram.prepare(zs);
    ctx = std::move(histogram.contexts(histogram.original_contexts));
    return Parent::start(ctx, s);
}

Error::Type Initialiser::start(Scene &s, Zones &zs) noexcept {
    return start(s, zs, contexts);
}

Error::Type Initialiser::process(VPP::Kernel::Histogram::Context &c, 
                                 Scene &scn) noexcept {
    ASSERT(c.original != nullptr,
           "Task::Kernel::Histogram::Initialiser::process(): Cannot initialise "
           "old contexts");
    c.initialise(scn.view);

    return Error::OK;
}


CamShift::CamShift(const int mode, VPP::Kernel::Histogram::Engine &e) noexcept
    : Parent(mode), contexts(), histogram(e), term() {}

Error::Type CamShift::start(Scene &s, 
                            VPP::Kernel::Histogram::Contexts &ctx) noexcept {
    /* Cache the right mode for the view */
    s.view.cache(histogram.mode());
    return Parent::start(ctx, s);
}

Error::Type CamShift::start(Scene &s) noexcept {
    contexts = std::move(histogram.contexts(histogram.history_contexts));
    return start(s, contexts);
}

Error::Type CamShift::process(VPP::Kernel::Histogram::Context &c,
                              Scene &scn) noexcept {
    if (c.zone().valid()) {
        auto dest  = std::move(c.back_project(scn.view));
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
