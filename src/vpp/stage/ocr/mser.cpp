/**
 *
 * @file      vpp/stage/ocr/mser.cpp
 *
 * @brief     This the VPP MSER stage for OCR
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

#include "vpp/stage/ocr/mser.hpp"

namespace VPP {
namespace Stage {
namespace OCR {

MSER::MSER() noexcept : ForScene(true), mser() {
    use("mser", mser);
}

}  // namespace Reader
}  // namespace Stage
}  // namespace VPP
