/**
 *
 * @file      vpp/task/tracker/histogram.cpp
 *
 * @brief     These are various tasks to efficiently use Histogram trackers in a
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
#include "vpp/task/tracker/histogram.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace Task {
namespace Tracker {
namespace Histogram {

Initialiser::Initialiser(const int mode, 
                         VPP::Tracker::Histogram::Engine &e) noexcept
    : Parent(mode), contexts(), histogram(e) {}

Error::Type Initialiser::start(Scene &s, Zones &zs, 
                               VPP::Tracker::Histogram::Contexts &ctx)
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

Error::Type Initialiser::process(VPP::Tracker::Histogram::Context &c, 
                                 Scene &scn) noexcept {
    ASSERT(c.original != nullptr,
           "Task::Tracker::Histogram::Initialiser::process(): Cannot initialise "
           "old contexts");
    c.initialise(scn.view);

    return Error::OK;
}

CamShift::CamShift(const int mode, VPP::Tracker::Histogram::Engine &e) noexcept
    : Parent(mode), contexts(), histogram(e),
      term(cv::TermCriteria::EPS|cv::TermCriteria::COUNT, 10, 1.0) {

   epsilon.denominate("epsilon")
          .describe("The desired accuracy in terms of CamShift displacement "
                    "under which the search algorithm stops")
          .characterise(Customisation::Trait::SETTABLE);
    epsilon.range(1e-3, 10.0);
    epsilon.trigger([this](const float &e) { 
                           return onEpsilonUpdate(e); });
    Customisation::Entity::expose(epsilon);
    epsilon = 1.0;

    iterations.denominate("iterations")
              .describe("The maximal number of iterations after which the "
                        "search algorithm stops")
              .characterise(Customisation::Trait::SETTABLE);
    iterations.range(1, 1000);
    iterations.trigger([this](const int &i) { 
                           return onIterationsUpdate(i); });
    Customisation::Entity::expose(iterations);
    iterations = 10;

    threshold.denominate("threshold")
             .describe("The minimal threshold for accepting an histogram match")
             .characterise(Customisation::Trait::SETTABLE);
    threshold.range(0.001, 1.0);
    Customisation::Entity::expose(threshold);
    threshold = 0.4;
}

Error::Type CamShift::start(Scene &s, 
                            VPP::Tracker::Histogram::Contexts &ctx) noexcept {
    /* Cache the right mode for the view */
    s.view.cache(histogram.mode());
    return Parent::start(ctx, s);
}

Error::Type CamShift::start(Scene &s) noexcept {
    contexts = std::move(histogram.contexts(histogram.history_contexts));
    return start(s, contexts);
}

Error::Type CamShift::process(VPP::Tracker::Histogram::Context &c,
                              Scene &scn) noexcept { 
    c.camshift(scn.view, term, threshold);
    return Error::OK;
}

Customisation::Error CamShift::onEpsilonUpdate(const float &e) noexcept {
    term = cv::TermCriteria(cv::TermCriteria::EPS|cv::TermCriteria::COUNT, 
                            iterations, e);
    return Customisation::Error::NONE;
}

Customisation::Error CamShift::onIterationsUpdate(const int &i) noexcept {
    term = cv::TermCriteria(cv::TermCriteria::EPS|cv::TermCriteria::COUNT, 
                            i, epsilon);
    return Customisation::Error::NONE;
}

}  // namespace Histogram
}  // namespace Tracker
}  // namespace Task
}  // namespace VPP
