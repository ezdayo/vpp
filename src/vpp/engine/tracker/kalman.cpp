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

Kalman::Model::Model() noexcept
    : cv::KalmanFilter(VPP::Zone::State::length, VPP::Zone::Measure::length,
                       0, CV_32F), validity(0), timeout(10.0) {}

Kalman::Model::Model(const Kalman::Model &other) noexcept
    : cv::KalmanFilter(), validity(0), timeout(other.timeout) {
#define KF_MATRIX_COPY(x) x = std::move(other.x.clone())
    KF_MATRIX_COPY(statePre);
    KF_MATRIX_COPY(statePost);
    KF_MATRIX_COPY(transitionMatrix);
    KF_MATRIX_COPY(controlMatrix);
    KF_MATRIX_COPY(measurementMatrix);
    KF_MATRIX_COPY(processNoiseCov);
    KF_MATRIX_COPY(measurementNoiseCov);
    KF_MATRIX_COPY(errorCovPre);
    KF_MATRIX_COPY(gain);
    KF_MATRIX_COPY(errorCovPost);
    KF_MATRIX_COPY(temp1);
    KF_MATRIX_COPY(temp2);
    KF_MATRIX_COPY(temp3);
    KF_MATRIX_COPY(temp4);
    KF_MATRIX_COPY(temp5);
}

Kalman::Model::~Model() noexcept = default;

bool Kalman::Model::predictability(float time) noexcept {
    if (time > 0) {
        timeout = time;
        return true;
    }
    return false;
}

bool Kalman::Model::valid() const noexcept {
    return validity > 0;
}

float Kalman::Model::accuracy() const noexcept {
    return std::max(validity, 0.0f)/timeout;
}

void Kalman::Model::invalidate() noexcept {
    validity = 0;
}


void Kalman::Model::predict(float dt, VPP::Zone::State &state) noexcept {
    if (valid()) {
        /* Set the time delta */
        transitionMatrix.at<float>(3)  = dt;
        transitionMatrix.at<float>(12) = dt;
        transitionMatrix.at<float>(21) = dt;

        cv::KalmanFilter::predict().copyTo(static_cast<cv::Mat&>(state));

        validity -= dt;
    }
}

void Kalman::Model::correct(VPP::Zone::State &state) noexcept {
    if (valid()) {
        cv::KalmanFilter::correct(state);
    } else {
        errorCovPre.at<float>(0)  = 1;
        errorCovPre.at<float>(9)  = 1; 
        errorCovPre.at<float>(18) = 1;
        errorCovPre.at<float>(27) = 1;
        errorCovPre.at<float>(36) = 1;
        errorCovPre.at<float>(45) = 1;
        errorCovPre.at<float>(54) = 1;
        errorCovPre.at<float>(63) = 1;

        state = static_cast<VPP::Zone::Measure>(state);

        statePost = state; 
    }

    validity = timeout;
}

#define EXPOSE_MATRIX(M, L, D) \
    M##L.denominate(#M#L)\
        .describe("Line " #L " of the " #D " matrix " #M)\
        .characterise(Customisation::Trait::CONFIGURABLE);\
    Customisation::Entity::expose(M##L)

Kalman::Kalman(Scene &history) noexcept 
    : VPP::Engine::ForScene(), predictability(10.0), tscale(1.0),
      F0({   1.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      F1({   0.0,   1.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      F2({   0.0,   0.0,   1.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      F3({   0.0,   0.0,   0.0,   1.0,   0.0,   0.0,   0.0,   0.0 }),
      F4({   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0,   0.0 }),
      F5({   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0 }),
      F6({   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0 }),
      F7({   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0 }),
      H0({   1.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      H1({   0.0,   1.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      H2({   0.0,   0.0,   1.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      H3({   0.0,   0.0,   0.0,   1.0,   0.0,   0.0,   0.0,   0.0 }),
      H4({   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0,   0.0 }),
      Q0({  1e-2,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      Q1({   0.0,  1e-2,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      Q2({   0.0,   0.0,  1e-2,   0.0,   0.0,   0.0,   0.0,   0.0 }),
      Q3({   0.0,   0.0,   0.0,  1e-2,   0.0,   0.0,   0.0,   0.0 }),
      Q4({   0.0,   0.0,   0.0,   0.0,  1e-2,   0.0,   0.0,   0.0 }),
      Q5({   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0,   0.0 }),
      Q6({   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0,   0.0 }),
      Q7({   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   0.0,   1.0 }),
      R0({   0.1,   0.0,   0.0,   0.0,   0.0 }),
      R1({   0.0,   0.1,   0.0,   0.0,   0.0 }),
      R2({   0.0,   0.0,   0.1,   0.0,   0.0 }),
      R3({   0.0,   0.0,   0.0,   0.1,   0.0 }),
      R4({   0.0,   0.0,   0.0,   0.0,   0.1 }),
      model(), tracked(), latest(history) {

    predictability.denominate("predictability")
                  .describe("The timeout after which a tracked object is no "
                            "longer estimated if not seen again")
                  .characterise(Customisation::Trait::SETTABLE);
    predictability.trigger([this](const float &t) { 
                           return onPredictabilityUpdate(t); });
    Customisation::Entity::expose(predictability);

    predictability.denominate("tscale")
                  .describe("The scaling factor for the time delta update in "
                            "the transition state matrix")
                  .characterise(Customisation::Trait::SETTABLE);
    Customisation::Entity::expose(predictability);

    EXPOSE_MATRIX(F, 0, "transition state");
    EXPOSE_MATRIX(F, 1, "transition state");
    EXPOSE_MATRIX(F, 2, "transition state");
    EXPOSE_MATRIX(F, 3, "transition state");
    EXPOSE_MATRIX(F, 4, "transition state");
    EXPOSE_MATRIX(F, 5, "transition state");
    EXPOSE_MATRIX(F, 6, "transition state");
    EXPOSE_MATRIX(F, 7, "transition state");

    EXPOSE_MATRIX(H, 0, "measure");
    EXPOSE_MATRIX(H, 1, "measure");
    EXPOSE_MATRIX(H, 2, "measure");
    EXPOSE_MATRIX(H, 3, "measure");
    EXPOSE_MATRIX(H, 4, "measure");

    EXPOSE_MATRIX(Q, 0, "process noise covariance");
    EXPOSE_MATRIX(Q, 1, "process noise covariance");
    EXPOSE_MATRIX(Q, 2, "process noise covariance");
    EXPOSE_MATRIX(Q, 3, "process noise covariance");
    EXPOSE_MATRIX(Q, 4, "process noise covariance");
    EXPOSE_MATRIX(Q, 5, "process noise covariance");
    EXPOSE_MATRIX(Q, 6, "process noise covariance");
    EXPOSE_MATRIX(Q, 7, "process noise covariance");
    
    EXPOSE_MATRIX(R, 0, "measures noise covariance");
    EXPOSE_MATRIX(R, 1, "measures noise covariance");
    EXPOSE_MATRIX(R, 2, "measures noise covariance");
    EXPOSE_MATRIX(R, 3, "measures noise covariance");
    EXPOSE_MATRIX(R, 4, "measures noise covariance");
}
        
Kalman::~Kalman() noexcept = default;

static Customisation::Error do_copy(std::vector<float> i, 
                                    cv::Mat &o, int r) noexcept {
    int sz = static_cast<int>(i.size());
    ASSERT((sz == o.cols) && (r >= 0) && (r < o.rows),
            "Engine::Tracker::Kalman::setup(): "
            "Cannot fit a %d row at row %d of a %x%d matrix",
            sz, r, o.cols, o.rows);

    if ((sz != o.cols) && (r >= o.rows)) {
        return Customisation::Error::INVALID_RANGE;
    }

    cv::Mat from(1, sz, CV_32F, static_cast<void *>(i.data()),
                 cv::Mat::AUTO_STEP);
    from.copyTo(o.row(r));

    return Customisation::Error::NONE;
}

Customisation::Error Kalman::setup() noexcept {
    Customisation::Error e;

    /* Update the model parametrisation with the current settings */
    e = do_copy(F0, model.transitionMatrix, 0);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(F1, model.transitionMatrix, 1);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(F2, model.transitionMatrix, 2);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(F3, model.transitionMatrix, 3);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(F4, model.transitionMatrix, 4);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(F5, model.transitionMatrix, 5);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(F6, model.transitionMatrix, 6);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(F7, model.transitionMatrix, 7);
    if (e != Customisation::Error::NONE) { return e; }

    e = do_copy(H0, model.measurementMatrix, 0);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(H1, model.measurementMatrix, 1);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(H2, model.measurementMatrix, 2);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(H3, model.measurementMatrix, 3);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(H4, model.measurementMatrix, 4);
    if (e != Customisation::Error::NONE) { return e; }

    e = do_copy(Q0, model.processNoiseCov, 0);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(Q1, model.processNoiseCov, 1);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(Q2, model.processNoiseCov, 2);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(Q3, model.processNoiseCov, 3);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(Q4, model.processNoiseCov, 4);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(Q5, model.processNoiseCov, 5);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(Q6, model.processNoiseCov, 6);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(Q7, model.processNoiseCov, 7);
    if (e != Customisation::Error::NONE) { return e; }

    e = do_copy(R0, model.measurementNoiseCov, 0);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(R1, model.measurementNoiseCov, 1);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(R2, model.measurementNoiseCov, 2);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(R3, model.measurementNoiseCov, 3);
    if (e != Customisation::Error::NONE) { return e; }
    e = do_copy(R4, model.measurementNoiseCov, 4);
    if (e != Customisation::Error::NONE) { return e; }
    
    tracked.clear();
    latest = std::move(Scene());

    return e;
}
        
Error::Type Kalman::process(Scene &scene) noexcept {
    /* Predict what is expected on this scene from the previous one */
    predict(scene);

    /* Update zones that can be mapped from previous to current scene by moving
     * the uuid of the old zone to the new mapped one, and update the
     * predictions by predicting with the oldest predictions, invalidate the
     * old zone. This can be done by zone.update(older); */

    /* Correct all zones of the new scene */
    for (auto &r : latest.zones()) {
        auto &z = r.get();
        auto it = predictor(z);
        auto &model = it->second;
        model.correct(z.state);
    }

    /* Append the still valid old zones to the current scene and empty the             latest scene */
    scene.update(latest);

    /* Keep the history */
    latest = std::move(scene.remember());

    return Error::NONE;
}

Customisation::Error Kalman::onPredictabilityUpdate(const float &t) noexcept {
    if(model.predictability(t)) {
        return Customisation::Error::NONE;
    } else {
        return Customisation::Error::INVALID_VALUE;
    }
}

std::unordered_map<uint64_t, Kalman::Model>::iterator
    Kalman::predictor(Zone &zone) noexcept {
    auto key = zone.uuid;
    auto it = tracked.find(key);

    if (it == tracked.end()) {
        auto p = tracked.emplace(key, model);
        it = p.first;
    }

    return it;
}

void Kalman::predict(Scene &scene) noexcept {
    if (!latest.empty()) {
        /* Compute the delta time in seconds */
        auto dt_ms = scene.timestamp() - latest.timestamp();
        float dt = static_cast<float>(dt_ms) / 1000.0f;

        for (auto &r : latest.zones()) {
            auto &z = r.get();
            if (z.invalid()) {
                continue;
            } 
            auto it = predictor(z);
            auto &model = it->second;
            model.predict(dt, z.state);
            if (model.valid()) {
                /* Update the zone spatial contents */
                auto centre   = z.state.centre;
                auto size     = z.state.size;
                cv::Point3f c = centre;
                cv::Point3f s(size.x/2, size.y/2, 0);
            
                auto tl       = latest.view.depth.project(c-s);
                auto br       = latest.view.depth.project(c+s);
                auto geom     = br - tl;

                z.x           = tl.x;
                z.y           = tl.y;
                z.width       = geom.x;
                z.height      = geom.y;
            } else {
                z.invalidate();
                tracked.erase(it);
            }
        }

        /* Store the untracked zones into an internal buffer */
        untracked = std::move(latest.extract(Zone::when_invalid));
    }
}

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
