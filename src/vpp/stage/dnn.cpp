/**
 *
 * @file      vpp/stage/dnn.cpp
 *
 * @brief     These are the VPP DNN stages implementations
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
#include "vpp/stage/dnn.hpp"

namespace VPP {
namespace Stage {
namespace DNN {

Detector::Detector() noexcept : ForScene(true) {
#ifdef VPP_HAS_OPENCV_DNN_SUPPORT
    use("ocv", ocv);
#endif
#ifdef VPP_HAS_DARKNET_SUPPORT
    use("darknet", darknet);
#endif
}

Classifier::Classifier() noexcept : ForZone(true) {
#ifdef VPP_HAS_OPENCV_DNN_SUPPORT
    use("ocv", ocv);
#endif
    filter = ([](const Scene &, const Zone &z) noexcept { 
                        return !VPP::DNN::Dataset::isText(z); });
}

}  // namespace DNN
}  // namespace Stage
}  // namespace VPP
