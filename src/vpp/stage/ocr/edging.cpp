/**
 *
 * @file      vpp/stage/ocr/edging.cpp
 *
 * @brief     This the VPP edging stage for OCR
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

#include "vpp/stage/ocr/edging.hpp"

namespace VPP {
namespace Stage {
namespace OCR {

Edging::Edging() noexcept : ForScene(true), engine() {
    use("engine", engine);
}

}  // namespace Reader
}  // namespace Stage
}  // namespace VPP
