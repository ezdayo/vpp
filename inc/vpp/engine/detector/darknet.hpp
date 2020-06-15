/**
 *
 * @file      vpp/engine/detector/darknet.hpp
 *
 * @brief     This is the VPP Darknet detector description file
 *
 * @details   This engine is a Darknet YOLO detector description capable of
 *            running any YOLO DNN
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

#include <opencv2/core/core.hpp>

#include "customisation/parameter.hpp"
#include "vpp/dnn/engine.hpp"

extern "C" {
#include "darknet/darknet.h"
}

namespace VPP {
namespace Engine {
namespace Detector {

class Darknet : public VPP::DNN::Engine::ForScene {
    public:
        Darknet() noexcept;
        ~Darknet() noexcept;

        Customisation::Error setup() noexcept override;
        Error::Type process(Scene &scene) noexcept override;
        void terminate() noexcept override;

        PARAMETER(Direct, Saturating, Immediate, float) hierarchy;
        PARAMETER(Direct, Saturating, Immediate, float) nms;

    private:
        std::string architecture, weights;
        ::network * net;
        int         input_w, input_h;
        cv::Mat     input_f, input_p[3];
        image       img_input, img_yolo;
};

}  // namespace Detector
}  // namespace Engine
}  // namespace VPP
