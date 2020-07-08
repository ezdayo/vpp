/**
 *
 * @file      vpp/kernel/kalman.hpp
 *
 * @brief     This is the VPP kalman kernel description file
 *
 * @details   This kernel is a kalman predicter aimed at predicting the next
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

#include "customisation.hpp"
#include "vpp/kernel.hpp"
#include "vpp/zone.hpp"

namespace VPP {
namespace Kernel {
namespace Kalman {

class Parameters : public cv::KalmanFilter {
    public:
        Parameters() noexcept 
            : cv::KalmanFilter(Zone::State::length, Zone::Measure::length, 0, 
                               CV_32F), timeout(10.0) {};
        Parameters(const Parameters &other) noexcept = default;        
        Parameters(Parameters &&other) noexcept = default;
        Parameters &operator=(const Parameters &other) = delete;
        Parameters &operator=(Parameters &&other) = delete;
        ~Parameters() noexcept = default;

        float timeout;
};

class Context : public VPP::Kernel::Context, cv::KalmanFilter {
    public:
        explicit Context(Zone &zone, Zone::Copier &copier,
                         unsigned int sz, Parameters &params) noexcept;
        ~Context() noexcept = default;

        /* Initialising the Karman filter (resetting the filters) */ 
        void initialise() noexcept;

        float accuracy() const noexcept;

        /* Stack the predicted zone atop */
        void predict(const VPP::View &view, float dt) noexcept;

        /* Use the zone atop for the correction if there are at least threshold
         * zones stacked. Usually two zones are mandatory for there must be
         * one for prediction and one for setting the right output.
         * stacked */
        void correct(unsigned int threshold = 2) noexcept;

    protected:
        float       validity;
        Parameters &config;
};

using Contexts = std::vector<std::reference_wrapper<Context>>;

class Engine : public VPP::Kernel::Engine<Engine, Context> {
    public:
        using Parent = VPP::Kernel::Engine<Engine, Context>;

        explicit Engine(const Zone::Copier &c, unsigned int sz = 2) noexcept;
        ~Engine() noexcept = default;

        Customisation::Error setup() noexcept override;

        Customisation::Error clear() noexcept override;

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
        
        void prepare(Zones &zs) noexcept;
                                   
    protected:
        Customisation::Error onPredictabilityUpdate(const float &t) noexcept;

        Parameters           model;
};

}  // namespace Kalman
}  // namespace Kernel
}  // namespace VPP
