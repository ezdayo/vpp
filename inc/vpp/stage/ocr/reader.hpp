/**
 *
 * @file      vpp/stage/ocr/reader.hpp
 *
 * @brief     These are the VPP OCR reader stages definitions
 *
 * @details   Here are all stages that embed OCR capabilities
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

#include "vpp/engine/ocr/tesseract.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {
namespace OCR {

class Reader : public Stage::ForZone  {
    public:
        Reader() noexcept;
        ~Reader() noexcept = default;

        VPP::Engine::OCR::Tesseract tesseract;
};

}  // namespace OCR
}  // namespace Stage
}  // namespace VPP
