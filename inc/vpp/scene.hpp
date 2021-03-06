/**
 *
 * @file      vpp/scene.hpp
 *
 * @brief     This is the VPP scene description file
 *
 * @details   This file describes the structure of the token that flows in the
 *            visual pipeline processor (VPP). This token is called a scene and
 *            consists of a timestamp, an image and a list of zones of interest.
 *            Each zone is a bounding box in the image with a context, a short
 *            description, an emergency flag, and a list of predictions.
 *            Each prediction is a score, a list index and an object index.
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

#include <cstdint>
#include <list>
#include <opencv2/core/core.hpp>
#include <vector>

#include "vpp/view.hpp"
#include "vpp/zone.hpp"

namespace VPP {

/** Scene type */
class Scene final {
    public:
        using ZoneFilter = 
            std::function<bool (const Zone &) noexcept>;

        Scene() noexcept;
        ~Scene() noexcept = default;

        /** Scenes can be moved and never copied */
        Scene(const Scene& other) = delete;
        Scene(Scene&& other) = default;
        Scene& operator=(const Scene& other) = delete;
        Scene& operator=(Scene&& other) = default;

        inline uint64_t ts_ms() const noexcept {
            return view.ts_ms();
        }

        inline bool broken() const noexcept {
            return view.empty();
        }

        inline bool empty() const noexcept {
            return areas.empty();
        }

        Zone &mark(Zone zone) noexcept;
        
        inline Zone &mark(BBox bbox) noexcept {
            /* Relying on copy-elision for performance */
            return mark(Zone(bbox));
        }

        inline Zone &mark(cv::Rect bbox) noexcept {
            /* Relying on copy-elision for performance */
            return mark(Zone(BBox(bbox)));
        }

        inline Zone &mark(cv::Rect_<float> bbox) noexcept {
            /* Relying on copy-elision for performance */
            return mark(Zone(BBox(bbox, view.frame())));
        }

        ConstZones zones() const noexcept;
        Zones zones() noexcept;

        Zones zones(const ZoneFilterDelegate &f) noexcept;
        ConstZones zones(const ZoneFilterDelegate &f) const noexcept;
        Zones zones(const ZoneFilter &f) noexcept;
        ConstZones zones(const ZoneFilter &f) const noexcept;

        std::list<Zone> extract(const ZoneFilterDelegate &f) noexcept;
        std::list<Zone> extract(const ZoneFilter &f) noexcept;

        /* Remembering a scene for tracking: everything is copied except the
         * images (useless) */
        Scene remember() const noexcept;

        /* The visual environment captured for the scene */
        View view;

    private:
        /* Using a double-linked list to ensure that zones keep their actual
         * locations unless removed from the scene, including when other zones
         * are appended or removed */
        std::list<Zone> areas;
};

}  // namespace VPP
