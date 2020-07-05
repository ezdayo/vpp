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
    : ForScene(), engine(Zone::Copy::All, 2),
      prediction(VPP::Task::Kernel::Kalman::Prediction::Mode::Async*8, engine), 
      correction(VPP::Task::Kernel::Kalman::Correction::Mode::Async*8, engine),
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
    auto dt_ms = scene.timestamp() - latest.timestamp();
    float dt_s = static_cast<float>(dt_ms) / 1000.0f;

    /* Perform the predictions as parallel tasks */
    prediction.start(scene, dt_s);
    auto e = prediction.wait();
    if (e != Error::NONE) {
        return e;
    }

    /* Do some new to old context mapping and merge ... */
    auto src = engine.contexts(engine.original_contexts);
    auto dst = engine.contexts(engine.history_contexts);
    e=matcher.estimate(src, dst);
    if (e != Error::NONE) {
        return e;
    }

    /* Correct Kalman predictions as parallel tasks */
    correction.start(scene);
    e = correction.wait();
    if (e != Error::NONE) {
        return e;
    }

    std::lock_guard<std::mutex> lock(update);
    engine.cleanup(entering, leaving);
    latest = std::move(scene.remember());
    return Error::NONE;
}

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
