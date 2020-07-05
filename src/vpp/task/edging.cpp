/**
 *
 * @file      vpp/task/edging.cpp
 *
 * @brief     This is a task for detecting the edges in a scene
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

#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <numeric>

#include "vpp/dnn/dataset.hpp"
#include "vpp/log.hpp"
#include "vpp/task/edging.hpp"
#include "vpp/vlog.hpp"

namespace VPP {
namespace Task {

Edging::Edging(const int mode) noexcept : Parent(mode) {
    
    input_scale.denominate("input_scale")
         .describe("Input scaling factor for accelerating edge detection")
         .characterise(Customisation::Trait::SETTABLE);
    expose(input_scale);
    input_scale.range(0, 16);
    input_scale = 2;

    blur_size.denominate("blur_size")
             .describe("The aperture size for the preprocess blur (1 element "
                       "for median blur and 2 elements for blur)")
             .characterise(Customisation::Trait::SETTABLE);
    expose(blur_size);
    blur_size.range(1, 16);
    blur_size = { 3, 3 };
    
    min_area.denominate("min_area")
            .describe("Minimal area for the detected edged-zones in per 1024 "
                      "of the image size")
            .characterise(Customisation::Trait::SETTABLE);
    expose(min_area);
    min_area.range(1, 1024);
    min_area = 16;

    threshold_low.denominate("threshold_low")
                 .describe("Canny edge-detector low thresold value")
                 .characterise(Customisation::Trait::SETTABLE);
    expose(threshold_low);
    threshold_low.range(0, 255);
    threshold_low = 85;

    threshold_high.denominate("threshold_high")
                  .describe("Canny edge-detector high thresold value")
                  .characterise(Customisation::Trait::SETTABLE);
    expose(threshold_high);
    threshold_high.range(0, 255);
    threshold_high = 255;

    kernel_size.denominate("kernel_size")
               .describe("The size of the Sobel kernel of the canny "
                         "edge-detector")
               .characterise(Customisation::Trait::SETTABLE);
    expose(kernel_size);
    kernel_size.range(1, 16);
    kernel_size = 3; 

    levels.denominate("levels")
          .describe("The number of threshold levels for canny "
                    "edge-detectors")
          .characterise(Customisation::Trait::SETTABLE);
    expose(levels);
    levels.range(1, 16);
    levels = 3; 
}

/* Checking if the angle cosine is below 0.3, i.e. the angle is close to
 * 90 degrees. This function calculates the square cosine and check that the
 * square cosine is below 0.09 which is close to 1/11 */
static bool is_nearly_squared(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
    int64_t dx1 = pt1.x - pt0.x;
    int64_t dy1 = pt1.y - pt0.y;
    int64_t dx2 = pt2.x - pt0.x;
    int64_t dy2 = pt2.y - pt0.y;
    auto num = (dx1*dx2 + dy1*dy2);
    auto den = (dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2);
                                    
    return 11*num*num < den;
}

Error::Type Edging::process(Scene &scene) noexcept {

    auto &input = scene.view.bgr().input();
    /* Scale the input image to accelerate detection */
    int scale = static_cast<int>(input_scale);
    if (scale == 0) { // Automatic scaling arrange for an ~57600 pixel output
        auto square_scale = input.total()/57600.0f;
        scale = static_cast<int>(std::round(std::sqrt(square_scale)));
    } else {          // Manual scaling
        scale = std::max(scale, 1);
    }
    cv::Mat scaled;

    if (scale != 1) {
        cv::resize(input, scaled, input.size()/scale);
    } else {
        scaled = input;
    }

    /* Blur the scaled image */
    cv::Mat blurred;
    const std::vector<int> &bs = static_cast<std::vector<int>>(blur_size);

    if ( (!bs.empty()) && (bs.size() < 3) &&
         (std::accumulate(bs.begin(), bs.end(), 0) > 0) ) {
        if (bs.size() == 1) {
            cv::medianBlur(scaled, blurred, bs.front());
        } else {
            cv::blur(scaled, blurred, cv::Size(bs.front(), bs.back()));
        }
    } else {
        blurred = scaled;
    }

    cv::Mat plane(blurred.size(), CV_8U), edged;
    std::vector<Contour> contours;
    Contour approx;
    
    /* Find edge boxes into every color plane of the image */
    for (int c = 0; c < 3; c++) {
        int ch[] = { c, 0 };
        cv::mixChannels(&blurred, 1, &plane, 1, ch, 1);

        /* Do a canny search at various levels */
        const int max_level = std::max(1, static_cast<int>(levels));
        for (int l=0; l < max_level; ++l) {
            cv::Mat canny;
            cv::Canny(plane, canny, (threshold_low*(l+1))/max_level, 
                      (threshold_high*(l+1))/max_level, kernel_size);
            if (edged.empty()) edged = canny;
            cv::bitwise_or(canny, edged, edged);
        }
    }
    
    DISPLAY("canny", edged);
#define HOUGHLINES
#ifdef HOUGHLINES
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edged, lines, 1, CV_PI/180, 20, 20, 0);
    for (auto &l : lines) {
#ifdef USE_HOUGHLINES
        cv::line(edged, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]),
                 cv::Scalar(255), 1, cv::LINE_AA);
#endif
        cv::line(scene.view.bgr().drawable(), cv::Point(l[0], l[1])*scale,
                 cv::Point(l[2], l[3])*scale, cv::Scalar(0, 0 ,255), 3, 
                 cv::LINE_AA);
    }
    SHOW("hough", scene);
#endif /* HOUGHLINES */

    cv::dilate(edged, edged, cv::Mat(), cv::Point(-1, -1));
    DISPLAY("dilated", edged);
            
    /* Get contours */
    cv::findContours(edged, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    auto area_threshold = (edged.total() * static_cast<int>(min_area))/1024;
    for (auto &c : contours) {
        cv::approxPolyDP(cv::Mat(c), approx,
                         cv::arcLength(cv::Mat(c), true)*0.02, true);
        auto area = fabs(cv::contourArea(cv::Mat(approx)));
        if ( (approx.size() == 4) && (area > area_threshold) &&
             cv::isContourConvex(approx) ) {
                    
            bool valid = true;
            for (int j = 2; j < 5; j++) {
                valid &= is_nearly_squared(approx[j%4], approx[j-2],
                                               approx[j-1]);
            }

            if (valid) {
                for (auto &p : approx) {
                    p = p * scale;
                }

                /* For debug purposes */
                {
                    const cv::Point* p = &approx[0];
                    int n = 4;
                    cv::polylines(scene.view.bgr().drawable(), &p, &n, 1, true,
                                  cv::Scalar(0,255,0), 3, cv::LINE_AA);
                }
    
                Zone z(approx);
                scene.mark(std::move(z)).context = Prediction(1.0f, 0, 50);
                SHOW("edging", scene);
            }
        }
    }

    return Error::NONE;
}

}  // namespace Task
}  // namespace VPP
