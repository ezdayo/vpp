/**
 *
 * @file      vpp/engine/tracker/camshift.hpp
 *
 * @brief     This is the VPP camshift-based predicter and tracker description
 *            file
 *
 * @details   This engine is a camshift predicter aimed at predicting the next
 *            estimated position of zones coupled to a histogram comparisons to
 *            check if the predicted zone is relevant or not
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

#include "vpp/engine/tracker/camshift.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

CamShift::CamShift(Scene &history, std::mutex &synchro, 
                   std::vector<Zone> *added,
                   std::vector<Zone> *removed) noexcept
    : ForScene(), engine(Zone::Copy::Geometry, 3),
      initialisation(Util::Task::Core::Mode::Async*8, engine), 
      estimation(Util::Task::Core::Mode::Async*8, engine),
      matcher(Matcher::Mode::Sync, Matcher::Mode::Async*8,
              Matcher::Mode::Async*8),
      update(synchro), latest(history), entering(added), 
      leaving(removed) {
    engine.denominate("engine");
    expose(engine);

    initialisation.denominate("initialisation");
    expose(initialisation);

    estimation.denominate("estimation");
    expose(estimation);

    matcher.denominate("matcher");
    expose(matcher);
}

Error::Type CamShift::process(Scene &scene) noexcept {

    /* Get the historic and new contexts lists */
    VPP::Kernel::Histogram::Contexts new_contexts;
    auto historic_contexts = 
        std::move(engine.contexts(engine.history_contexts));
    
    /* Initialise the new zones histograms (and new contexts) in parallel 
     * tasks */
    auto zones = std::move(scene.zones());
    initialisation.start(scene, zones, new_contexts);
    auto e = initialisation.wait();
    if (e != Error::NONE) {
        return e;
    }
    
    /* And estimate the new position of historic contexts */
    estimation.start(scene, historic_contexts);
    e = estimation.wait();
    if (e != Error::NONE) {
        return e;
    }

    /* Do some new to old context mapping and merge ... */
    e=matcher.estimate(new_contexts, historic_contexts);
    if (e != Error::NONE) {
        return e;
    }

    /* Get the matches */
    auto matches = matcher.extract(true, true);
    for (auto &m : matches) {
        matcher.destination(m).merge(matcher.source(m));
    }
    
    /* Keep track of the changes! */
    std::lock_guard<std::mutex> lock(update);
    engine.cleanup(scene, entering, leaving);
    latest = std::move(scene.remember());
    
    return Error::NONE;
}

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP