/**
 *
 * @file      vpp/stage/dnn.hpp
 *
 * @brief     These are the VPP DNN stages definitions
 *
 * @details   Here are all stages that embed DNN for detection, classification
 *            or any other neural network related to the VPP
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

#ifdef VPP_HAS_DARKNET_SUPPORT
#include "vpp/engine/detector/darknet.hpp"
#endif
#ifdef VPP_HAS_OPENCV_DNN_SUPPORT
#include "vpp/engine/classifier/ocv.hpp"
#include "vpp/engine/detector/ocv.hpp"
#endif
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {
namespace DNN {

class Detector : public Stage::ForScene {
    public:
        Detector() noexcept;
        ~Detector() noexcept = default;

#ifdef VPP_HAS_DARKNET_SUPPORT
        VPP::Engine::Detector::Darknet darknet;
#endif
#ifdef VPP_HAS_OPENCV_DNN_SUPPORT
        VPP::Engine::Detector::OCV     ocv;
#endif
};

class Classifier : public Stage::ForZone {
    public:
        Classifier() noexcept;
        ~Classifier() noexcept = default;

#ifdef VPP_HAS_OPENCV_DNN_SUPPORT
        VPP::Engine::Classifier::OCV ocv;
#endif
};

}  // namespace DNN
}  // namespace Stage
}  // namespace VPP
