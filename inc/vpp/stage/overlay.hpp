/**
 *
 * @file      vpp/stage/overlay.hpp
 *
 * @brief     This is the VPP overlay handling stage definition
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

#include "vpp/engine/overlay.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {
namespace Overlay {

using AAMode            = VPP::Engine::Overlay::AAMode;
using DrawingStyle      = VPP::Engine::Overlay::DrawingStyle;
using LayerStyle        = VPP::Engine::Overlay::LayerStyle;
using TextStyle         = VPP::Engine::Overlay::TextStyle;
using ZoneStyle         = VPP::Engine::Overlay::ZoneStyle;
using ZoneStylist       = VPP::Engine::Overlay::ZoneStylist;
using ZoneStyleDelegate = VPP::Engine::Overlay::ZoneStyleDelegate;

using Font              = VPP::Engine::Overlay::Font;
using Layer             = VPP::Engine::Overlay::Layer;
using Logo              = VPP::Engine::Overlay::Logo;

template <typename ...Z> class Core : public VPP::Core::Stage<Z...> {
    public:
        Core() noexcept;
        ~Core() noexcept = default;

        VPP::Engine::Overlay::Core<Z...> ocv;
};

/* Describing a stage for handling a full scene */
using ForScene = Core<>;

/* Describing a stage for handling a single zone in a scene */
using ForZone = Core<Zone>;

/* Describing a stage for handling multiple zones in a scene */
using ForZones = Core<Zones>;


}  // namespace Overlay
}  // namespace Stage
}  // namespace VPP
