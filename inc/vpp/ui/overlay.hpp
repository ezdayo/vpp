/**
 *
 * @file      vpp/ui/overlay.hpp
 *
 * @brief     This is the defintion of the VPP-specific overlay UI
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

#include "vpp/log.hpp"
#include "vpp/util/ocv/overlay.hpp"
#include "vpp/scene.hpp"

namespace VPP {
namespace UI {

class Overlay : public Util::OCV::Overlay {

    public:
        using AAMode       = Util::OCV::Overlay::AAMode;
        using DrawingStyle = Util::OCV::Overlay::DrawingStyle;
        using LayerStyle   = Util::OCV::Overlay::LayerStyle;
        using TextStyle    = Util::OCV::Overlay::TextStyle;
        using Font         = Util::OCV::Overlay::Font;
        using Layer        = Util::OCV::Overlay::Layer;

        using Util::OCV::Overlay::draw;

        struct ZoneStyle final {
            DrawingStyle box;
            TextStyle    text;
            bool         adaptColor;
        };

        class ZoneStyleDelegate {
            public:
                virtual ZoneStyle getStyle(const VPP::Zone &zone, 
                                           const ZoneStyle &baseStyle) 
                    const noexcept = 0;
            };
        
        using ZoneStylist = 
            std::function<ZoneStyle (const VPP::Zone &,
                                     const ZoneStyle &) noexcept>;
 
        Overlay() noexcept;
        ~Overlay() noexcept;

        /* Resetting default styles */
        void resetDefaultZoneStyle() noexcept;

        /* Zone drawing primitives */
        void draw(cv::Mat &frame, const VPP::Zone &zone)
            const noexcept;
        void draw(cv::Mat &frame, const VPP::Zone &zone,
                  const ZoneStyle &style) const noexcept;
        void draw(cv::Mat &frame, const VPP::Zone &zone,
                  const ZoneStyleDelegate &delegate) const noexcept;
        void draw(cv::Mat &frame, const VPP::Zone &zone,
                  const ZoneStylist &stylist) const noexcept;
        void draw(cv::Mat &frame, const VPP::Zone &zone,
                  const ZoneStyle &style, const ZoneStyleDelegate &delegate) 
            const noexcept;
        void draw(cv::Mat &frame, const VPP::Zone &zone,
                  const ZoneStyle &style, const ZoneStylist &stylist) 
            const noexcept;

        /* Scene drawing primitives */
        void draw(cv::Mat &frame, const VPP::Scene &scn)
            const noexcept;
        void draw(cv::Mat &frame, const VPP::Scene &scn,
                  const ZoneStyle &style) const noexcept;
        void draw(cv::Mat &frame, const VPP::Scene &scn,
                  const ZoneStyleDelegate &delegate) const noexcept;
        void draw(cv::Mat &frame, const VPP::Scene &scn,
                  const ZoneStylist &stylist) const noexcept;
        void draw(cv::Mat &frame, const VPP::Scene &scn,
                  const ZoneStyle &style, const ZoneStyleDelegate &delegate)
            const noexcept;
        void draw(cv::Mat &frame, const VPP::Scene &scn,
                  const ZoneStyle &style, const ZoneStylist &stylist)
            const noexcept;

        /* Default style */
        ZoneStyle defaultZoneStyle;

    private:
        static ZoneStyle defaultZoneStylist(const VPP::Zone &zone, 
                                            const ZoneStyle &baseStyle)
            noexcept;
};

} // namespace UI
} // namespace VPP
