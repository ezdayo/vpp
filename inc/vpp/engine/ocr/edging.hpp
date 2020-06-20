/**
 *
 * @file      vpp/engine/ocr/edging.hpp
 *
 * @brief     This is an edging engine aimed at detecting areas for OCR
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

#include "customisation/parameter.hpp"
#include "vpp/error.hpp"
#include "vpp/engine.hpp"
#include "vpp/scene.hpp"
#include "vpp/task/edging.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace Engine {
namespace OCR {

class Edging : public Engine::ForScene {
    public:
        Edging() noexcept;
        ~Edging() noexcept = default;

        Error::Type process(Scene &scene) noexcept override;

    private:
        VPP::Task::Edging  detector;
};

}  // namespace OCR
}  // namespace Engine
}  // namespace VPP
