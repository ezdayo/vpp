/**
 *
 * @file      vpp/stage/ocr/edging.hpp
 *
 * @brief     These is the VPP OCR edging stage definition
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

#include "vpp/engine/ocr/edging.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {
namespace OCR {

class Edging : public Stage::ForScene  {
    public:
        Edging() noexcept;
        ~Edging() noexcept = default;

        VPP::Engine::OCR::Edging engine;
};

}  // namespace OCR
}  // namespace Stage
}  // namespace VPP
