/**
 *
 * @file      vpp/stage/input.hpp
 *
 * @brief     This is the VPP input stage definition
 *
 * @details   This is always the first stage used in any VPP pipeline for it is
 *            the only one to get the original stream of images, be it from an
 *            internal camera, a wifi camera, any other network video stream, or
 *            even from the output of another pipeline.
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

#include "vpp/engine/bridge.hpp"
#include "vpp/engine/capture.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {

template <typename ...Z> class Input : public Core::Stage<Z...> {
    public:
        Input() noexcept;
        ~Input() noexcept = default;

        VPP::Engine::Bridge<Z...> bridge;
};

template <> class Input<> : public Core::Stage<> {
    public:
        Input() noexcept;
        ~Input() noexcept = default;

        VPP::Engine::Bridge<> bridge;
        VPP::Engine::Capture  capture;
};

/* Describing a stage input for handling a full scene */
using InputForScene = Input<>;

/* Describing a stage input for handling squentially single zones in a scene */
using InputForZone = Input<Zone>;

/* Describing a stage input for handling multiple zones in a scene */
using InputForZones = Input<Zones>;

}  // namespace Stage
}  // namespace VPP
