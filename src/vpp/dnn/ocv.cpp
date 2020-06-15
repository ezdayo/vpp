/**
 *
 * @file      vpp/dnn/ocv.cpp
 *
 * @brief     This is the generic VPP OCV DNN engine implementation file
 *
 * @details   This is the implementation of a generic VPP OCV DNN engine.
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

#include "vpp/log.hpp"
#include "vpp/dnn/ocv.hpp"

namespace VPP {
namespace DNN {
namespace Engine {

template <typename ...Z> OCV<Z...>::OCV() noexcept 
    : Core<Z...>(), size(), RGB(false), mean(), scale(1.0f),
      architecture(""), weights(""), net() {

        size.denominate("size")
            .describe("The input size for the OCV DNN")
            .characterise(Customisation::Trait::CONFIGURABLE);
        Customisation::Entity::expose(size);

        RGB.denominate("RGB")
           .describe("Are the OCV DNN inputs in RGB mode?")
           .characterise(Customisation::Trait::CONFIGURABLE);
        RGB.use(Customisation::Translator::BoolFormat::NO_YES);
        Customisation::Entity::expose(RGB);

        mean.denominate("mean")
            .describe("The mean vector to substract to inputs for the OCV DNN")
            .characterise(Customisation::Trait::CONFIGURABLE);
        Customisation::Entity::expose(mean);

        scale.denominate("scale")
              .describe("The input scaling factor for the OCV DNN")
              .characterise(Customisation::Trait::CONFIGURABLE);
        Customisation::Entity::expose(scale);
}

template <typename ...Z> Customisation::Error OCV<Z...>::setup() noexcept {
    std::string net_architecture = OCV<Z...>::network.architecture;
    std::string net_weights      = OCV<Z...>::network.weights;

    if ( (architecture != net_architecture) || (weights != net_weights) ) {
        terminate();
        
        auto offset_vec = static_cast<std::vector<float> >(mean);
        if (offset_vec.size() > 0) {
            if (offset_vec.size() == 1) {
                offset = cv::Scalar(offset_vec[0]);
            } else if (offset_vec.size() == 3) {
                offset = cv::Scalar(offset_vec[0], offset_vec[1],
                                    offset_vec[2]);
            } else {
                LOGE("%s[%s]::setup(): Wrong mean scalar provided: it shall "
                     "be 0, 1 or 3 element vector!",
                     OCV<Z...>::value_to_string().c_str(), 
                     OCV<Z...>::name().c_str());
                return Customisation::Error::INVALID_VALUE;
            }
        }

        net = cv::dnn::readNet(net_weights, net_architecture, "");
        if (net.empty()) {
            LOGE("%s[%s]::setup(): Cannot load OpenCV DNN with config '%s' "
                 "and weights '%s'",
                 OCV<Z...>::value_to_string().c_str(),
                 OCV<Z...>::name().c_str(),
                 net_architecture.c_str(), net_weights.c_str());
            return Customisation::Error::INVALID_VALUE;
        }

        architecture = std::move(net_architecture);
        weights      = std::move(net_weights);

        net.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_OPENCL_FP16); 
    }

    return Customisation::Error::NONE;
}

template <typename ...Z> void OCV<Z...>::terminate() noexcept {
    if (! net.empty()) {
        net = cv::dnn::Net();
        architecture.clear();
        weights.clear();        
    }
}

/* Create template implementations */
template class OCV<>;
template class OCV<Zone>;

}  // namespace Engine
}  // namespace DNN
}  // namespace VPP
