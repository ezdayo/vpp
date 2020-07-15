/**
 *
 * @file      vpp/task/motion.hpp
 *
 * @brief     This is a dense optical flow estimator using the Gunnar Farneback 
 *            algorithm 
 *
 * @details   This engine is a dense flow estimator, i.e. a pixel-based motion
 *            estimator, meant to create the motion view of the scene, a Vx/Vy
 *            two channel matrix estimating the speed of each pixel.
 *            It allows the prediction of the current scene zones from the
 *            previous one.
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

#include "customisation/entity.hpp"
#include "vpp/error.hpp"
#include "vpp/scene.hpp"

namespace VPP {
namespace Task {

class Motion : public Parametrisable {
    public: 

        Motion(Scene &history) noexcept;
        ~Motion() noexcept = default;
        
        Error::Type estimate(Scene &scene) noexcept;

        /* The scale to apply on both directions to build the pyramid */
        PARAMETER(Direct, Saturating, Immediate, double) scale;

        /* The number of pyramid layers excluding the original image */
        PARAMETER(Direct, Saturating, Immediate, int)    layers;

        /* The averaging window size, the bigger the window, the best motion
         * detection is */
        PARAMETER(Direct, Saturating, Immediate, int)    window;

        /* The number of iterations at each pyramid level */
        PARAMETER(Direct, Saturating, Immediate, int)    iterations;

        /* The size of the pixel neighborhood used to find polynomial expansion
         * at each pixel */
        PARAMETER(Direct, Saturating, Immediate, int)    neighbourhood;

        /* The standard deviation of the Gaussian that is used to smooth 
         * derivatives for the polynomial expansion at each pixel */
        PARAMETER(Direct, Saturating, Immediate, double) sigma;

    private:
        Scene &latest;
};

}  // namespace Task
}  // namespace VPP
