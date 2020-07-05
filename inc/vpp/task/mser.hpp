/**
 *
 * @file      vpp/task/mser.hpp
 *
 * @brief     This is the maximally stable extremal region (MSER) task
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

#include "vpp/config.hpp"
#ifndef VPP_HAS_FEATURE_DETECTION_SUPPORT
# error ERROR: VPP does not support OpenCV feature detection!
#endif

#include <opencv2/features2d.hpp>
#include <string>
#include <vector>

#include "customisation/parameter.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"
#include "vpp/scene.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace Task {

class MSER : public Task::Single<MSER, Scene&> {
    public:
        using Parent = VPP::Task::Single<MSER, Scene&>;
        using typename Parent::Mode;

        explicit MSER(const int mode) noexcept;
        ~MSER() noexcept = default;

        Customisation::Error setup() noexcept override;
        void terminate() noexcept override;

        PARAMETER(Direct, None, Immediate, int)    delta;
        PARAMETER(Direct, None, Immediate, int)    min_area;
        PARAMETER(Direct, None, Immediate, int)    max_area;
        PARAMETER(Direct, None, Immediate, double) max_variation;
        PARAMETER(Direct, None, Immediate, double) min_diversity;
        PARAMETER(Direct, None, Immediate, int)    max_evolution;
        PARAMETER(Direct, None, Immediate, double) threshold_area;
        PARAMETER(Direct, None, Immediate, double) min_margin;
        PARAMETER(Direct, None, Immediate, int)    edge_blur_size;

        std::function<bool (const cv::Mat &img, const cv::Rect &,
                            const std::vector<cv::Point> &contour) 
                      noexcept> filter;

        Error::Type process(Scene &scene) noexcept;

    private:
        /* OpenCV MSER shared smart pointer */
        cv::Ptr<cv::MSER> core;
};

}  // namespace Task
}  // namespace VPP
