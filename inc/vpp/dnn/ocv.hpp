/**
 *
 * @file      vpp/dnn/ocv.hpp
 *
 * @brief     This is the VPP DNN OCV engine description file
 *
 * @details   This is the definition of the VPP OpenCV (OCV) DNN engine, a class
 *            that gathers any generic information for describing an OCV DNN 
 *            engine.
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

#include "vpp/config.hpp"
#ifndef VPP_HAS_OPENCV_DNN_SUPPORT
# error ERROR: VPP does not support the OpenCV DNN!
#endif

#include <opencv2/dnn.hpp>

#include "customisation/parameter.hpp"
#include "vpp/dnn/engine.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace DNN {
namespace Engine {

template <typename ...Z> class OCV : public Core<Z...> {
    public:
        OCV() noexcept;
        virtual ~OCV() noexcept = default; 

        Customisation::Error setup() noexcept override;
        void terminate() noexcept override;

        VPP::Size                                                 size;
        PARAMETER(Direct, None, Immediate, bool)                  RGB;
        PARAMETER(Direct, Bounded, Immediate, std::vector<float>) mean;
        PARAMETER(Direct, Bounded, Immediate, float)              scale;

    protected:
        std::string  architecture, weights;
        cv::dnn::Net net;
        cv::Scalar   offset;
};

}  // namespace Engine
}  // namespace DNN
}  // namespace VPP
