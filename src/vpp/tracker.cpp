/**
 *
 * @file      vpp/tracker.cpp
 *
 * @brief     This is a 3D capable Kalman filter tracker implementation
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

#include "vpp/scene.hpp"

#include "vpp/log.hpp"

namespace VPP {
namespace Tracker {

/* Keep a default state instance for fast copy initialisation */
State State::DEFAULT;

State::State() noexcept : 
            cv::KalmanFilter(length, Measure::length, 0, CV_32F), 
            centre(), speed(), size(),
            state(length, 1, CV_32F, &centre.x, cv::Mat::AUTO_STEP),
            validity(0), timeout(10) {

    // Transition state matrix A
    // [ 1 0 0  dt 0  0  0 0 ]
    // [ 0 1 0  0  dt 0  0 0 ]
    // [ 0 0 1  0  0  dt 0 0 ]
    // [ 0 0 0  1  0  0  0 0 ]
    // [ 0 0 0  0  1  0  0 0 ]
    // [ 0 0 0  0  0  1  0 0 ]
    // [ 0 0 0  0  0  0  1 0 ]
    // [ 0 0 0  0  0  0  0 1 ]
    cv::setIdentity(transitionMatrix);
    
    // Measure Matrix H
    // [ 1 0 0 0 0 0 0 0 ]
    // [ 0 1 0 0 0 0 0 0 ]
    // [ 0 0 1 0 0 0 0 0 ]
    // [ 0 0 0 0 0 0 1 0 ]
    // [ 0 0 0 0 0 0 0 1 ]
    measurementMatrix = cv::Mat::zeros(Measure::length, length, CV_32F);
    measurementMatrix.at<float>(0)  = 1.0f;
    measurementMatrix.at<float>(9)  = 1.0f;
    measurementMatrix.at<float>(18) = 1.0f;
    measurementMatrix.at<float>(30) = 1.0f;
    measurementMatrix.at<float>(39) = 1.0f;
    
    // Process Noise Covariance Matrix Q
    // [ Ex   0    0    0     0     0    0    0  ]
    // [ 0    Ey   0    0     0     0    0    0  ]
    // [ 0    0    Ez   0     0     0    0    0  ]
    // [ 0    0    0    Ev_x  0     0    0    0  ]
    // [ 0    0    0    0     Ev_y  0    0    0  ]
    // [ 0    0    0    0     0     Ev_z 0    0  ]
    // [ 0    0    0    0     0     0    Ew   0  ]
    // [ 0    0    0    0     0     0    0    Eh ]
    cv::setIdentity(processNoiseCov, cv::Scalar(1e-2));
    processNoiseCov.at<float>(27) = 5.0f;
    processNoiseCov.at<float>(36) = 5.0f;
    processNoiseCov.at<float>(45) = 5.0f;

    // Measures Noise Covariance Matrix R
    cv::setIdentity(measurementNoiseCov, cv::Scalar(1e-1));
}

State::State(const State &other) noexcept : 
            cv::KalmanFilter(),
            centre(other.centre), speed(other.speed), size(other.size),
            state(length, 1, CV_32F, &centre.x, cv::Mat::AUTO_STEP),
            validity(other.validity), timeout(other.timeout) {
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

State &State::operator = (const Measure &measure) noexcept {
    validity = 0;
    correct(measure);
    return *this;
}

float State::similarity(const State &other) const noexcept {
    auto s = size.iou(other.size);
    if (s > 0) {
        auto d = centre.square_dist(other.centre);
        if (d < 1e-6) {
            d = 1e-6;
        }
        return s/d;
    }

    return 0;
}
 

void State::predictability(float time) noexcept {
    if (time > 0) {
        timeout = time;
    }
}

void State::predict(float dt) noexcept {
    if (valid()) {
        /* Set the time delta */
        transitionMatrix.at<float>(3)  = dt;
        transitionMatrix.at<float>(12) = dt;
        transitionMatrix.at<float>(21) = dt;

        cv::KalmanFilter::predict().copyTo(state);
        validity -= dt;
    }
}

void State::correct(const Measure &measure) noexcept {
    if (valid()) {
        cv::KalmanFilter::correct(measure);
    } else {
 
        errorCovPre.at<float>(0)  = 1;
        errorCovPre.at<float>(9)  = 1; 
        errorCovPre.at<float>(18) = 1;
        errorCovPre.at<float>(27) = 1;
        errorCovPre.at<float>(36) = 1;
        errorCovPre.at<float>(45) = 1;
        errorCovPre.at<float>(54) = 1;
        errorCovPre.at<float>(63) = 1;

        centre = measure.centre;
        speed  = Util::OCV::Triplet();
        size   = measure.size;
        
        statePost = *this; 
    }
    
    /* Once corrected, validity is improved in all cases */ 
    validity = timeout;
}

}  // namespace Tracker
}  // namespace VPP
