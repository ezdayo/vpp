/**
 *
 * @file      vpp/engine/classifier/ocv.hpp
 *
 * @brief     This is the VPP OCV DNN classifier description file
 *
 * @details   This is an engine for running any OpenCV (OCV) DNN classifier
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
namespace Classifier {

class OCV : public VPP::DNN::Engine::OCV<Zone> {
    public:
        OCV() noexcept;
        ~OCV() noexcept;

        Error::Type process(Scene &scene, Zone &zone) noexcept override;
};

}  // namespace Classifier
}  // namespace Engine
}  // namespace VPP
