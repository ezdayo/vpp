/**
 *
 * @file      vpp/task/blur.cpp
 *
 * @brief     These are various tasks to manage blur with images
 *
 * @details   This is a collection of tasks for managing images that are blurred
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

#include <opencv2/imgproc.hpp>

#include "vpp/log.hpp"
#include "vpp/task/blur.hpp"

namespace VPP {
namespace Task {
namespace Blur {

Skipping::Skipping(const int mode) noexcept : VPP::Tasks::Tiled(mode) {
    sharpness.denominate("sharpness")
             .describe("The minimum sharpness level to consider a tile as "
                       "not being blurred")
             .characterise(Customisation::Trait::CONFIGURABLE);
    expose(sharpness);

    coverage.denominate("coverage")
            .describe("The minimal ratio of non blurred tiles not to skip the "
                      "scene ")
            .characterise(Customisation::Trait::CONFIGURABLE);
    expose(coverage);
}

Error::Type Skipping::start(Scene &s, cv::Rect &frame) noexcept {
    tiles_valid = 0;

    return VPP::Tasks::Tiled::start(s, frame);
}

Error::Type Skipping::wait() noexcept {
    auto error = VPP::Tasks::Tiled::wait();

    if (error < Error::OK) {
        return error;
    }

    if (tiles_valid < tiles_total * coverage) {
        return Error::RETRY;
    }
        
    return error;
}

Error::Type Skipping::process(Scene &s, cv::Rect &r) noexcept {
    /* Background information on this algorithm is provided at:
     * https://www.pyimagesearch.com/2015/09/07/blur-detection-with-opencv */
    cv::Mat input, grey, mean, laplacian, stddev;

    /* Crop the ROI and move it to grey to compute the Laplacian */
    /*cv::cvtColor(s.view.bgr(r).input(), grey, cv::COLOR_BGR2GRAY);
    cv::Laplacian(grey, laplacian, CV_16S, 3, 1, 0, cv::BORDER_DEFAULT);*/
    cv::Laplacian(s.view.image(Image::Mode::GRAY, r).input(), laplacian, 
                  CV_16S, 3, 1, 0, cv::BORDER_DEFAULT);
    
    /* Compute the standard deviation which first element is an estimation
     * of its sharpness */
    cv::meanStdDev(laplacian, mean, stddev);

    auto level = stddev.at<double>(0, 0);
    level = level * level; 

    if (level >= sharpness) {
        std::lock_guard<std::mutex> lock(synchro);
        ++ tiles_valid;
    }

    return Error::OK;
}

}  // namespace Blur
}  // namespace Task
}  // namespace VPP
