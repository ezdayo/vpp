/**
 *
 * @file      vpp/engine/tracker/kalman.cpp
 *
 * @brief     This is the VPP kalman-based tracker implementation file
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

#include <opencv2/core/mat.hpp>

#include "vpp/engine/tracker/kalman.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

Kalman::Kalman(Scene &history, std::mutex &synchro, std::vector<Zone> *added,
               std::vector<Zone> *removed) noexcept
    : ForScene(), engine(Zone::Copy::Geometry, 3),
      prediction(VPP::Task::Tracker::Kalman::Prediction::Mode::Async*8, engine), 
      correction(VPP::Task::Tracker::Kalman::Correction::Mode::Async*8, engine),
      matcher(Matcher::Mode::Sync, Matcher::Mode::Async*8,
              Matcher::Mode::Async*8),
      update(synchro), latest(history), entering(added), 
      leaving(removed) {
    engine.denominate("engine");
    expose(engine);

    prediction.denominate("prediction");
    expose(prediction);

    correction.denominate("correction");
    expose(correction);

    matcher.denominate("matcher");
    expose(matcher);
}

Customisation::Error Kalman::setup() noexcept {
    latest = std::move(Scene());

    return Customisation::Error::NONE;
}

Error::Type Kalman::process(Scene &scene) noexcept {

    /* Add the new zones to the kalam trackers */
    auto zones = std::move(scene.zones());
    engine.prepare(zones);

    /* Compute the delta time in seconds */
    auto dt_ms = scene.ts_ms() - latest.ts_ms();
    float dt_s = static_cast<float>(dt_ms) / 1000.0f;
    
    /* Perform the predictions as parallel tasks */
    auto historic_contexts = 
        std::move(engine.contexts(engine.history_contexts));
    prediction.start(scene, dt_s, historic_contexts);
    auto e = prediction.wait();
    if (e != Error::NONE) {
        return e;
    }
    
    /* Do some new to old context mapping and merge ... */
    auto new_contexts = engine.contexts(engine.original_contexts);
    e=matcher.estimate(new_contexts, historic_contexts);
    if (e != Error::NONE) {
        return e;
    }

    /* Get the matches */
    auto matches = matcher.extract(true, true);
    for (auto &m : matches) {
        matcher.destination(m).merge(matcher.source(m));
    }
    
    /* Correct Kalman predictions as parallel tasks */
    correction.start(scene, historic_contexts);
    e = correction.wait();
    if (e != Error::NONE) {
        return e;
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
