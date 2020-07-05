/**
 *
 * @file      vpp/engine/ocr/mser.hpp
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

#pragma once

#include "vpp/error.hpp"
#include "vpp/engine.hpp"
#include "vpp/scene.hpp"
#include "vpp/task/mser.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace Engine {
namespace OCR {

class MSER : public Engine::ForScene {
    public:
        MSER() noexcept;
        ~MSER() noexcept = default;

        Error::Type process(Scene &scene) noexcept override;

    private:
        VPP::Task::MSER  detector;
};

}  // namespace OCR
}  // namespace Engine
}  // namespace VPP
