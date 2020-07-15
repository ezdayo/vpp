/**
 *
 * @file      vpp/task/motion.hpp
 *
 * @brief     This is a dense optical flow estimator using the Gunnar Farneback 
 *            algorithm 
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

#include "customisation.hpp"
#include "vpp/task/motion.hpp"

namespace VPP {
namespace Task {

Motion::Motion(Scene &history) noexcept 
    : Customisation::Entity("Task"), latest(history) {
    scale.denominate("scale")
         .describe("the scale to apply on both directions to build the pyramid")
         .characterise(Customisation::Trait::SETTABLE);
    scale.range(0.01, 0.99);
    Customisation::Entity::expose(scale);
    scale = 0.5;

    layers.denominate("layers")
          .describe("the number of pyramid layers excluding the original image")
          .characterise(Customisation::Trait::SETTABLE);
    layers.range(0, 10);
    Customisation::Entity::expose(layers);
    layers = 3;

    window.denominate("window")
          .describe("the averaging window size, the bigger the window, the "
                    "better motion detection is")
          .characterise(Customisation::Trait::SETTABLE);
    window.range(1, 128);
    Customisation::Entity::expose(window);
    window = 15;

    iterations.denominate("iterations")
              .describe("the number of iterations at each pyramid level")
              .characterise(Customisation::Trait::SETTABLE);
    iterations.range(1, 16);
    Customisation::Entity::expose(iterations);
    iterations = 3;

    neighbourhood.denominate("neighbourhood")
                 .describe("the size of the pixel neighborhood used to find "
                           "polynomial expansion at each pixel")
                 .characterise(Customisation::Trait::SETTABLE);
    neighbourhood.range(1, 9);
    Customisation::Entity::expose(neighbourhood);
    neighbourhood = 5;

    sigma.denominate("sigma")
         .describe("the standard deviation of the Gaussian that is used to "
                   "smooth derivatives for the polynomial expansion at each "
                   "pixel")
         .characterise(Customisation::Trait::SETTABLE);
    sigma.range(0.1, 2);
    Customisation::Entity::expose(sigma);
    sigma = 1.2;
}

Error::Type Motion::estimate(Scene &scene) noexcept {
    scene.view.cache(VPP::Image::Mode::GRAY);
    if (latest.view.empty()) {
        return Error::NONE;
    }

    cv::Mat flow, gray, old_gray;
    auto old_flow = latest.view.cached_motion();
    cv::resize(scene.view.gray().input(), gray, scene.view.frame().size()/2);
    if (old_flow != nullptr) {
        old_flow->input().copyTo(flow);
    } else {
        flow = std::move(cv::Mat(gray.size(), CV_32FC2));
    }

    cv::resize(latest.view.gray().input(), old_gray, 
               scene.view.frame().size()/2);

    cv::calcOpticalFlowFarneback(old_gray, gray, flow, 
                                 static_cast<double>(scale),
                                 static_cast<int>(layers)+1, 
                                 static_cast<int>(window),
                                 static_cast<int>(iterations),
                                 static_cast<int>(neighbourhood), 
                                 static_cast<double>(sigma),
                                 cv::OPTFLOW_USE_INITIAL_FLOW);

    scene.view.use(std::move(flow), VPP::Image::Mode::MOTION);

    return Error::NONE;
}

}  // namespace Task
}  // namespace VPP
