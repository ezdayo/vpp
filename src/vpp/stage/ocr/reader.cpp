/**
 *
 * @file      vpp/stage/ocr/reader.cpp
 *
 * @brief     This the VPP OCR stage
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


#include "vpp/dnn/dataset.hpp"
#include "vpp/stage/ocr/reader.hpp"

namespace VPP {
namespace Stage {
namespace OCR {

Reader::Reader() noexcept : ForZone(true) {
#ifdef VPP_HAS_TESSERACT_SUPPORT
    use("tesseract", tesseract);
#endif
    filter = ([](const Scene &, const Zone &z) noexcept { 
                        return VPP::DNN::Dataset::isText(z); });
}

}  // namespace Reader
}  // namespace Stage
}  // namespace VPP
