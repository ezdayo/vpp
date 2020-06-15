/**
 *
 * @file      vpp/engine/ocr/tesseract.hpp
 *
 * @brief     This is the Tesseract OCR engine
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

#include <string>
#include <tesseract/baseapi.h>

#include "customisation/parameter.hpp"
#include "error.hpp"
#include "vpp/engine.hpp"
#include "vpp/scene.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace Engine {
namespace OCR {

class Tesseract : public Engine::ForZone {
    public:
        Tesseract() noexcept;
        ~Tesseract() noexcept = default;

        Customisation::Error setup() noexcept override;
        Error::Type process(Scene &scene, Zone &zone) noexcept override;
        void terminate() noexcept override;

        PARAMETER(Direct, None, Immediate, std::string) path;
        PARAMETER(Direct, None, Immediate, std::string) language;
        PARAMETER(Direct, Bounded, Immediate, int)      oem;
        PARAMETER(Direct, Bounded, Immediate, int)      psm;

    private:
        std::string               current_path, current_language;
        tesseract::OcrEngineMode  current_oem;
        tesseract::PageSegMode    current_psm;
        tesseract::TessBaseAPI    tess;
};

}  // namespace OCR
}  // namespace Engine
}  // namespace VPP
