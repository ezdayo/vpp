/**
 *
 * @file      vpp/engine/overlay.cpp
 *
 * @brief     This engine overlays different informations on a scene
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
#include "vpp/logo.hpp"
#include "vpp/engine/overlay.hpp"

namespace VPP {
namespace Engine {
namespace Overlay {

static VPP::UI::Overlay::ZoneStyle 
    default_style(const VPP::Zone & /*zone*/, const ZoneStyle &base) noexcept {
    return base;
}

Logo::Logo() noexcept : Entity("Logo"), layer(), show(false), at() {
    /* Use the default VPP logo */
    layer.set({ VPP::Logo::width, VPP::Logo::height },
                VPP::Logo::bgr, VPP::Logo::alpha);

    show.denominate("show")
        .describe("Displaying the logo on frame?")
        .characterise(Customisation::Trait::CONFIGURABLE);
    show.use(Customisation::Translator::BoolFormat::NO_YES);
    expose(show);

    at.denominate("at")
      .describe("The relative location for the logo");
    expose(at);
}

template <typename ...Z> Core<Z...>::Core() noexcept
    : overlay(), style(), logo(), stylist(nullptr), styles() {
    style.denominate("style")
         .describe("The style for displaying zone informations")
         .characterise(Customisation::Trait::CONFIGURABLE);
    style.trigger([this](const std::string &s) { 
                     return onStyleUpdate(s); });
    Customisation::Entity::expose(style);

    define("default", default_style);
    style = "default";

    logo.denominate("logo");
    Customisation::Entity::expose(logo);
}

template <typename ...Z> 
Error::Type Core<Z...>::process(Scene &scene, Z&... /*z*/) noexcept {
    /* Draw the scene */
    auto &bgr = scene.view.bgr();
    bgr.flush();
    overlay.draw(bgr.drawable(), scene, *stylist);

    if ( (!logo.layer.empty()) && (logo.show) ) {
        overlay.draw(bgr.drawable(), logo.layer, logo.at);
    }

    return Error::NONE;
}

template <typename ...Z> 
void Core<Z...>::define(std::string sname, ZoneStylist s) noexcept {
    auto found = styles.find(sname);

    if (found != styles.end()) {
        LOGE("%s[%s]::define(): Style '%s' is already defined in the overlay",
             Customisation::Entity::value_to_string().c_str(),
             Customisation::Entity::name().c_str(),
             sname.c_str());
        return;
    }

    styles.emplace(sname, std::move(s));
    style.allow(std::move(sname));
}

template <typename ...Z> 
Customisation::Error Core<Z...>::onStyleUpdate(const std::string &s) noexcept {
    ASSERT(styles.find(s) != styles.end(),
           "%s[%s]::OnStyleUpdate(): Style '%s' is unknown!",
           Customisation::Entity::value_to_string().c_str(),
           Customisation::Entity::name().c_str(), s.c_str());
    stylist = &styles[s];

    return Customisation::Error::NONE;
}

/* Create template implementations */
template class Core<>;
template class Core<Zone>;
template class Core<Zones>;

}  // namespace Overlay
}  // namespace Engine
}  // namespace VPP
