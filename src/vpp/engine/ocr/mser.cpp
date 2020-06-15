/**
 *
 * @file      vpp/engine/ocr/mser.cpp
 *
 * @brief     This is a MSER engine aimed at detecting areas for OCR
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

#include <opencv2/imgproc/imgproc.hpp>

#include "vpp/log.hpp"
#include "vpp/engine/ocr/mser.hpp"

namespace VPP {
namespace Engine {
namespace OCR {

static bool keep_letters(const cv::Mat &/*img*/, const cv::Rect &zone,
                         const std::vector<cv::Point> &contour) noexcept {
    /* BBOX compactness (not too wide, not too tall) */
    if ( (zone.width > 3*zone.height) || (zone.height > 3*zone.width)) {
        return false;
    }

    /* Eccentricity */
    cv::RotatedRect ellipse = cv::fitEllipse(contour);
    auto eccentricity = ellipse.size.height / ellipse.size.width;
    if (eccentricity > 1.3) {
         return false;
    }
   
    /* Solidity */ 
    std::vector<cv::Point> hull;
    cv::convexHull(contour, hull);
    auto solidity = cv::contourArea(contour) / cv::contourArea(hull);
    if (solidity < 0.3) {
        return false;
    }

    return true;
}

MSER::MSER() noexcept
    : ForScene(), detector(Task::ForScene::Mode::Sync) {
    detector.denominate("detector");
    expose(detector);

    detector.delta          = 4;
    detector.min_area       = 64;
    detector.max_area       = 14400;
    detector.max_variation  = 0.25;
    detector.min_diversity  = 0.2;
    detector.max_evolution  = 200;
    detector.threshold_area = 2.0;
    detector.min_margin     = 0.003;
    detector.edge_blur_size = 5;

    detector.filter = keep_letters;
}

Error::Type MSER::process(Scene &scene) noexcept {
    detector.start(scene);
    auto e = detector.wait();

    return e;
}

}  // namespace OCR
}  // namespace Engine
}  // namespace VPP
