/**
 *
 * @file      vpp/engine/detector/ocv.hpp
 *
 * @brief     This is the VPP OCV DNN detector description file
 *
 * @details   This is an engine for running any OpenCV (OCV) DNN detector
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

#include "vpp/dnn/ocv.hpp"

namespace VPP {
namespace Engine {
namespace Detector {

class OCV : public VPP::DNN::Engine::OCV<> {
    public:
        OCV() noexcept;
        ~OCV() noexcept;

        Customisation::Error setup() noexcept override;
        Error::Type process(Scene &scene) noexcept override;
        void terminate() noexcept override;

        PARAMETER(Direct, Saturating, Immediate, float) nms;

    private:
        std::vector<cv::String>  names;
        std::vector<int>         outLayers;
        std::string              outLayerType;
        bool                     needsResizing;
        cv::Mat                  imInfo;
};

}  // namespace Detector
}  // namespace Engine
}  // namespace VPP
