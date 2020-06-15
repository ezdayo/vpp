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

Detector::Detector() noexcept : ForScene(true), darknet(), ocv() {
    use("ocv", ocv);
    use("darknet", darknet);
}

Classifier::Classifier() noexcept : ForZone(true), ocv() {
    use("ocv", ocv);
    filter = ([](const Scene &, const Zone &z) noexcept { 
                        return !VPP::DNN::Dataset::isText(z); });
}

}  // namespace DNN
}  // namespace Stage
}  // namespace VPP
