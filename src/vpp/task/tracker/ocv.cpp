/**
 *
 * @file      vpp/task/tracker/ocv.hpp
 *
 * @brief     These are various tasks to efficiently use OCV trackers in a scene
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
#include "vpp/task/tracker/ocv.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace Task {
namespace Tracker {
namespace OCV {

Initialiser::Initialiser(const int mode, 
                         VPP::Tracker::OCV::Engine &e) noexcept
    : Parent(mode), contexts(), ocv(e) {}

Error::Type Initialiser::start(Scene &s, Zones &zs, 
                               VPP::Tracker::OCV::Contexts &ctx)
    noexcept {
    ocv.prepare(zs);
    ctx = std::move(ocv.contexts(ocv.original_contexts));
    return Parent::start(ctx, s);
}

Error::Type Initialiser::start(Scene &s, Zones &zs) noexcept {
    return start(s, zs, contexts);
}

Error::Type Initialiser::process(VPP::Tracker::OCV::Context &c, 
                                 Scene &scn) noexcept {
    ASSERT(c.original != nullptr,
           "Task::Tracker::OCV::Initialiser::process(): Cannot initialise "
           "old contexts");
    c.initialise(scn.view);

    return Error::OK;
}

Predicter::Predicter(const int mode, VPP::Tracker::OCV::Engine &e) noexcept
    : Parent(mode), contexts(), ocv(e) {}

Error::Type Predicter::start(Scene &s, 
                            VPP::Tracker::OCV::Contexts &ctx) noexcept {
    return Parent::start(ctx, s);
}

Error::Type Predicter::start(Scene &s) noexcept {
    contexts = std::move(ocv.contexts(ocv.history_contexts));
    return start(s, contexts);
}

Error::Type Predicter::process(VPP::Tracker::OCV::Context &c,
                               Scene &scn) noexcept { 
    c.predict(scn.view);
    return Error::OK;
}

}  // namespace OCV
}  // namespace Tracker
}  // namespace Task
}  // namespace VPP
