/**
 *
 * @file      vpp/engine/ocr/edging.cpp
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

#include <opencv2/imgproc/imgproc.hpp>

#include "vpp/log.hpp"
#include "vpp/engine/ocr/edging.hpp"

namespace VPP {
namespace Engine {
namespace OCR {

Edging::Edging() noexcept
    : ForScene(), detector(Task::ForScene::Mode::Sync) {
    detector.denominate("detector");
    expose(detector);

    detector.input_scale    = 0;
    detector.blur_size      = { 3, 3 };
    detector.min_area       = 64;
    detector.threshold_low  = 30;
    detector.threshold_high = 120;
    detector.kernel_size    = 3;
    detector.levels         = 1;
}

Error::Type Edging::process(Scene &scene) noexcept {
    detector.start(scene);
    auto e = detector.wait();

    return e;
}

}  // namespace OCR
}  // namespace Engine
}  // namespace VPP
