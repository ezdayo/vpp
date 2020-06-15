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

#include "vpp/engine/classifier/ocv.hpp"
#include "vpp/engine/detector/darknet.hpp"
#include "vpp/engine/detector/ocv.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {
namespace DNN {

class Detector : public Stage::ForScene {
    public:
        Detector() noexcept;
        ~Detector() noexcept = default;

        VPP::Engine::Detector::Darknet darknet;
        VPP::Engine::Detector::OCV     ocv;
};

class Classifier : public Stage::ForZone {
    public:
        Classifier() noexcept;
        ~Classifier() noexcept = default;

        VPP::Engine::Classifier::OCV ocv;
};

}  // namespace DNN
}  // namespace Stage
}  // namespace VPP
