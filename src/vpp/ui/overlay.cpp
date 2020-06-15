/**
 *
 * @file      vpp/ui/overlay.cpp
 *
 * @brief     This is the implementation of the VPP-specific overlay UI
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

#include <unordered_map>

#include "vpp/ui/overlay.hpp"

namespace VPP {
namespace UI {

Overlay::Overlay() noexcept : Util::OCV::Overlay() {
    resetDefaultZoneStyle();
}

Overlay::~Overlay() noexcept = default;

void Overlay::resetDefaultZoneStyle() noexcept {
    defaultZoneStyle.box        = defaultDrawingStyle;
    defaultZoneStyle.text       = defaultTextStyle;
    defaultZoneStyle.adaptColor = false;
}

void Overlay::draw(cv::Mat &frame,
                   const VPP::Zone &zone) const noexcept {
    return draw(frame, zone, defaultZoneStyle);
}

void Overlay::draw(cv::Mat &frame, const VPP::Zone &zone,
                   const Overlay::ZoneStyle &style) const noexcept {
    return draw(frame, zone, style, defaultZoneStylist);
}

void Overlay::draw(cv::Mat &frame, const VPP::Zone &zone,
                   const Overlay::ZoneStyleDelegate &delegate) const noexcept {
    return draw(frame, zone, defaultZoneStyle, delegate);
}

void Overlay::draw(cv::Mat &frame, const VPP::Zone &zone,
                   const Overlay::ZoneStylist &stylist) const noexcept {
    return draw(frame, zone, defaultZoneStyle, stylist);
}

void Overlay::draw(cv::Mat &frame, const VPP::Zone &zone,
                   const Overlay::ZoneStyle &baseStyle,
                   const Overlay::ZoneStyleDelegate &delegate) const noexcept {

    auto style = delegate.getStyle(zone, baseStyle);

    if ( (style.adaptColor) && (style.box.thickness <= 0) ) {
        style.box.color[3] += (255 - style.box.color[3]) * 
                              (1.0 - zone.context.score);
    }

    Util::OCV::Overlay::draw(frame, zone, style.box);
    Util::OCV::Overlay::draw(frame, zone.description,
                             (zone.tl() + zone.br())/2,
                             style.text);
}

void Overlay::draw(cv::Mat &frame, const VPP::Zone &zone,
                   const Overlay::ZoneStyle &baseStyle,
                   const Overlay::ZoneStylist &stylist) const noexcept {

    auto style = stylist(zone, baseStyle);

    if ( (style.adaptColor) && (style.box.thickness <= 0) ) {
        style.box.color[3] += (255 - style.box.color[3]) * 
                              (1.0 - zone.context.score);
    }

    Util::OCV::Overlay::draw(frame, zone, style.box);
    Util::OCV::Overlay::draw(frame, zone.description,
                             (zone.tl() + zone.br())/2,
                             style.text);
}

void Overlay::draw(cv::Mat &frame,
                   const VPP::Scene &scn) const noexcept {
    return draw(frame, scn, defaultZoneStyle, defaultZoneStylist);
}

void Overlay::draw(cv::Mat &frame,
                   const VPP::Scene &scn,
                   const Overlay::ZoneStyle &style) const noexcept {
    return draw(frame, scn, style, defaultZoneStylist);
}

void Overlay::draw(cv::Mat &frame, const VPP::Scene &scn,
                   const Overlay::ZoneStyleDelegate &delegate) const noexcept {
    return draw(frame, scn, defaultZoneStyle, delegate);
}

void Overlay::draw(cv::Mat &frame, const VPP::Scene &scn,
                   const Overlay::ZoneStylist &stylist) const noexcept {
    return draw(frame, scn, defaultZoneStyle, stylist);
}

void Overlay::draw(cv::Mat &frame, const VPP::Scene &scn,
                   const Overlay::ZoneStyle &style,
                   const Overlay::ZoneStyleDelegate &delegate) const noexcept {
    for (auto const &zone : scn.zones()) {
        draw(frame, zone.get(), style, delegate);
    }
}

void Overlay::draw(cv::Mat &frame, const VPP::Scene &scn,
                   const Overlay::ZoneStyle &style,
                   const Overlay::ZoneStylist &stylist) const noexcept {
    for (auto const &zone : scn.zones()) {
        draw(frame, zone.get(), style, stylist);
    }
}

Overlay::ZoneStyle 
    Overlay::defaultZoneStylist(const VPP::Zone &/*zone*/, 
                                const Overlay::ZoneStyle &baseStyle) noexcept {
    /* Copy elision */
    return baseStyle;
}

} // namespace UI
} // namespace VPP

