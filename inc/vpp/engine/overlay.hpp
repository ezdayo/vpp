/**
 *
 * @file      vpp/engine/overlay.hpp
 *
 * @brief     This engine overlays different informations on a scene
 *
 * @details   This is a "scene-level" engine for drawing zones, text, images...
 *            on the displayable view of a scene.
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

#include <string>
#include <unordered_map>

#include "customisation/parameter.hpp"
#include "vpp/error.hpp"
#include "vpp/engine.hpp"
#include "vpp/scene.hpp"
#include "vpp/types.hpp"
#include "vpp/ui/overlay.hpp"

namespace VPP {
namespace Engine {
namespace Overlay {
 
using AAMode            = VPP::UI::Overlay::AAMode;
using DrawingStyle      = VPP::UI::Overlay::DrawingStyle;
using LayerStyle        = VPP::UI::Overlay::LayerStyle;
using TextStyle         = VPP::UI::Overlay::TextStyle;
using ZoneStyle         = VPP::UI::Overlay::ZoneStyle;
using ZoneStylist       = VPP::UI::Overlay::ZoneStylist;
using ZoneStyleDelegate = VPP::UI::Overlay::ZoneStyleDelegate;

using Font              = VPP::UI::Overlay::Font;
using Layer             = VPP::UI::Overlay::Layer;

class Logo : public Customisation::Entity {
    public:
        Logo() noexcept;
        ~Logo() noexcept = default;

        VPP::UI::Overlay::Layer                  layer;
        PARAMETER(Direct, None, Immediate, bool) show;
        VPP::Offset                              at;
};

template <typename ...Z> class Core : public VPP::Core::Engine<Z...> {
    public:

        Core() noexcept;
        ~Core() noexcept = default;

        Error::Type process(Scene &scene, Z&...z) noexcept override;

        void define(std::string name, ZoneStylist s) noexcept;

        VPP::UI::Overlay                                         overlay;
        PARAMETER(Direct, WhiteListed, Callable, std::string)    style;
        Logo                                                     logo;

    private:
        Customisation::Error onStyleUpdate(const std::string &s) noexcept;

        ZoneStylist *                                stylist;
        std::unordered_map<std::string, ZoneStylist> styles;
};

/* Describing an engine for handling a full scene */
using ForScene = Core<>;

/* Describing an engine for handling a single zone in a scene */
using ForZone = Core<Zone>;

/* Describing an engine for handling multiple zones in a scene */
using ForZones = Core<Zones>;

}  // namespace Overlay
}  // namespace Engine
}  // namespace VPP
