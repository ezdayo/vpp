/**
 *
 * @file      vpp/tracker/kalman.cpp
 *
 * @brief     This is the VPP kalman tracker implementation file
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

#include "vpp/tracker/kalman.hpp"

namespace VPP {
namespace Tracker {
namespace Kalman {

Context::Context(Zone &zone, Zone::Copier &copier,
                 unsigned int sz, Parameters &params) noexcept
    : VPP::Tracker::Context(zone, copier, sz), 
      cv::KalmanFilter(Zone::State::length, Zone::Measure::length, 0, CV_32F),
      validity(params.timeout), config(params) {
          initialise();
}

void Context::initialise() noexcept {
#define KF_MATRIX_COPY(x) x = std::move(config.x.clone())
    KF_MATRIX_COPY(statePre);
    statePost = std::move(static_cast<cv::Mat&>(zone().state).clone());
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

float Context::accuracy() const noexcept {
    return std::max(validity, 0.0f)/config.timeout;
}

void Context::predict(const VPP::View &view, float dt) noexcept {
    if (valid() && (original == nullptr)) {
        /* Set the time delta */
        transitionMatrix.at<float>(5)  = dt;
        transitionMatrix.at<float>(14) = dt;
        transitionMatrix.at<float>(23) = dt;
    
        /* Duplicate the previous zone and predict it !*/
        auto &z = stack(zone(-1));
        cv::KalmanFilter::predict().copyTo(static_cast<cv::Mat&>(z.state));
        z.project(view);

        validity -= dt;
    }
}

void Context::correct(unsigned int threshold) noexcept {
    if (zones.size() > threshold) {
        cv::KalmanFilter::correct(static_cast<Zone::Measure>(zone(-1).state));
        validity = config.timeout;
    } else {
        if (validity < 0) {
            invalidate();
        }
    } 
}

#define EXPOSE_MATRIX(M, L, D) \
    M##L.denominate(#M#L)\
        .describe("Line " #L " of the " #D " matrix " #M)\
        .characterise(Customisation::Trait::CONFIGURABLE);\
    Customisation::Entity::expose(M##L)

Engine::Engine(const Zone::Copier &c, unsigned int sz) noexcept 
    : VPP::Tracker::Engine<Engine, Context>(c, sz), predictability(10.0),
      tscale(1.0),
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
      model() {

    predictability.denominate("predictability")
                  .describe("The timeout after which a tracked object is no "
                            "longer estimated if not seen again")
                  .characterise(Customisation::Trait::SETTABLE);
    predictability.trigger([this](const float &t) { 
                           return onPredictabilityUpdate(t); });
    Customisation::Entity::expose(predictability);

    tscale.denominate("tscale")
                  .describe("The scaling factor for the time delta update in "
                            "the transition state matrix")
                  .characterise(Customisation::Trait::SETTABLE);
    Customisation::Entity::expose(tscale);

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
        
static Customisation::Error do_copy(std::vector<float> i, 
                                    cv::Mat &o, int r) noexcept {
    int sz = static_cast<int>(i.size());
    ASSERT((sz == o.cols) && (r >= 0) && (r < o.rows),
            "Tracker::Kalman::Engine::setup(): "
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

Customisation::Error Engine::setup() noexcept {
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
   
    setIdentity(model.errorCovPost, cv::Scalar::all(1));

    return clear(); 
}
    
Customisation::Error Engine::clear() noexcept {
    storage.clear();
    return Customisation::Error::NONE;
}

void Engine::prepare(Zones &zs) noexcept {
    Parent::prepare(zs, model);
}


Customisation::Error Engine::onPredictabilityUpdate(const float &t) noexcept {
    if (t > 0) {
        model.timeout = t;
        return Customisation::Error::NONE;
    } else {
        return Customisation::Error::INVALID_VALUE;
    }
}

}  // namespace Kalman
}  // namespace Tracker
}  // namespace VPP
