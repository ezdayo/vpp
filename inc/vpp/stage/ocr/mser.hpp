/**
 *
 * @file      vpp/stage/ocr/mser.hpp
 *
 * @brief     These is the VPP OCR MSER stage definition
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

#include "vpp/engine/ocr/mser.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {
namespace OCR {

class MSER : public Stage::ForScene  {
    public:
        MSER() noexcept;
        ~MSER() noexcept = default;

        VPP::Engine::OCR::MSER mser;
};

}  // namespace OCR
}  // namespace Stage
}  // namespace VPP
