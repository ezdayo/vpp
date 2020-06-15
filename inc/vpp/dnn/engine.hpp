/**
 *
 * @file      vpp/dnn/engine.hpp
 *
 * @brief     This is the VPP DNN engine description file
 *
 * @details   This is the definition of the VPP DNN engine, a class that gathers
 *            any generic information for describing a DNN engine.
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

#include "customisation/parameter.hpp"
#include "vpp/dnn/dataset.hpp"
#include "vpp/dnn/setup.hpp"
#include "vpp/core/engine.hpp"

namespace VPP {
namespace DNN {
namespace Engine {

template <typename ...Z> class Core : public VPP::Core::Engine<Z...> {
    public:
        Core() noexcept;
        virtual ~Core() noexcept = default; 

        std::string label(const Zone &zone) const noexcept;

        VPP::DNN::Dataset                               dataset;
        VPP::DNN::Setup                                 network;
        PARAMETER(Direct, Saturating, Immediate, float) threshold;
};

/* Describing an engine for handling a full scene */
using ForScene = Core<>;

/* Describing an engine for handling a single zone in a scene */
using ForZone = Core<Zone>;

/* Describing an engine for handling multiple zones in a scene */
using ForZones = Core<Zones>;

}  // namespace Engine
}  // namespace DNN
}  // namespace VPP
