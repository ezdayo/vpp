/**
 *
 * @file      vpp/dnn/engine.cpp
 *
 * @brief     This is the generic VPP DNN engine implementation file
 *
 * @details   This is the implementation of a generic VPP DNN engine.
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

#include "vpp/dnn/engine.hpp"

namespace VPP {
namespace DNN {
namespace Engine {

template <typename ...Z> Core<Z...>::Core() noexcept 
    : VPP::Core::Engine<Z...>(), dataset(), network(), threshold(0.4f) {

        dataset.denominate("dataset")
               .describe("The network dataset configuration file")
               .characterise(Customisation::Trait::CONFIGURABLE);
        Customisation::Entity::expose(dataset);

        network.denominate("network")
               .describe("The network configuration files")
               .characterise(Customisation::Trait::CONFIGURABLE);
        Customisation::Entity::expose(network);

        threshold.denominate("threshold")
                 .describe("The minimal threshold for keeping a prediction")
                 .characterise(Customisation::Trait::SETTABLE);
        threshold.range(0.0f, 1.0f);
        Customisation::Entity::expose(threshold);
}

template <typename ...Z>
std::string Core<Z...>::label(const Zone &zone) const noexcept {
    return dataset.label(zone, threshold);
}

/* Create template implementations */
template class Core<>;
template class Core<Zone>;
template class Core<Zones>;

}  // namespace Engine
}  // namespace DNN
}  // namespace VPP
