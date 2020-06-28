/**
 *
 * @file      vpp/engine/tracker/kalman.hpp
 *
 * @brief     This is the VPP kalman-based predicter and tracker description
 *            file
 *
 * @details   This engine is a kalman predicter aimed at predicting the next
 *            estimated position of zones coupled to a tracker to keep the last
 *            scene status
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

#pragma once

#include <opencv2/video/tracking.hpp>
#include <unordered_map>

#include "vpp/engine.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

class Kalman : public VPP::Engine::ForScene {
    private:
        class Model : public cv::KalmanFilter {
            public:
                Model() noexcept;
                Model(const Model &other) noexcept;
                ~Model() noexcept;

                bool predictability(float time) noexcept;
                bool valid() const noexcept;
                float accuracy() const noexcept;

                void invalidate() noexcept;
                void predict(float dt, VPP::Zone::State &state) noexcept;
                void correct(VPP::Zone::State &state) noexcept;

            private:
                float validity;
                float timeout;
        };

    public:
        explicit Kalman(Scene &history) noexcept;
        ~Kalman() noexcept;

        Customisation::Error setup() noexcept override;
        Error::Type process(Scene &scene) noexcept override;

        /* Predictability timeout: the time after which a tracked object is
         * no longer estimated if not seen again */
        PARAMETER(Direct, None, Callable, float)               predictability;

        /* Time scaler : the scale for time update in the transition state
         * matrix */
        PARAMETER(Direct, None, Immediate, float)              tscale;

        /* Transition state matrix F (8x8) */
        PARAMETER(Direct, None, Immediate, std::vector<float>) F0;
        PARAMETER(Direct, None, Immediate, std::vector<float>) F1;
        PARAMETER(Direct, None, Immediate, std::vector<float>) F2;
        PARAMETER(Direct, None, Immediate, std::vector<float>) F3;
        PARAMETER(Direct, None, Immediate, std::vector<float>) F4;
        PARAMETER(Direct, None, Immediate, std::vector<float>) F5;
        PARAMETER(Direct, None, Immediate, std::vector<float>) F6;
        PARAMETER(Direct, None, Immediate, std::vector<float>) F7;

        /* Measure matrix H (5x8) */ 
        PARAMETER(Direct, None, Immediate, std::vector<float>) H0;
        PARAMETER(Direct, None, Immediate, std::vector<float>) H1;
        PARAMETER(Direct, None, Immediate, std::vector<float>) H2;
        PARAMETER(Direct, None, Immediate, std::vector<float>) H3;
        PARAMETER(Direct, None, Immediate, std::vector<float>) H4;

        /* Process noise covariance matrix Q (8x8) */
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q0;
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q1;
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q2;
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q3;
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q4;
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q5;
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q6;
        PARAMETER(Direct, None, Immediate, std::vector<float>) Q7;

        /* Measures noise covariance matrix R (5x5) */
        PARAMETER(Direct, None, Immediate, std::vector<float>) R0;
        PARAMETER(Direct, None, Immediate, std::vector<float>) R1;
        PARAMETER(Direct, None, Immediate, std::vector<float>) R2;
        PARAMETER(Direct, None, Immediate, std::vector<float>) R3;
        PARAMETER(Direct, None, Immediate, std::vector<float>) R4;

    private:
        Customisation::Error onPredictabilityUpdate(const float &t) noexcept;
        std::unordered_map<uint64_t, Model>::iterator
                                    predictor(Zone &z) noexcept;
        void predict(Scene &scene) noexcept;

        Model                               model;
        std::unordered_map<uint64_t, Model> tracked;
        std::vector<Zone>                   untracked;
        Scene &                             latest;
};

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
