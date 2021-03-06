/**
 *
 * @file      vpp/scene.cpp
 *
 * @brief     This is the VPP scene implementation file
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

#include <chrono>
#include <ctime>
#include <functional>
#include <iterator>

#include "vpp/scene.hpp"

namespace VPP {

Scene::Scene() noexcept : view(), areas() { }

Zone &Scene::mark(Zone zone) noexcept {
    static uint64_t next_uuid = 0;
    static Zone     invalid;
 
    /* Crop the zone to prevent any issue and discard outside zones */
    static_cast<cv::Rect &>(zone) = zone & view.frame(); 
    if ((zone.width <= 0 ) || (zone.height <= 0)) {
        invalid = std::move(zone);
        invalid.invalidate();
        return invalid;
    }

    if (zone.uuid == 0) {
        zone.uuid = ++next_uuid;
        /* Update the zone state contents */
        zone.deproject(view);
    }

    areas.emplace_back(std::move(zone));

    return areas.back();
}
        
ConstZones Scene::zones() const noexcept {
    return ConstZones(areas.cbegin(), areas.cend());
}

Zones Scene::zones() noexcept {
    return Zones(areas.begin(), areas.end());
}

Zones Scene::zones(const ZoneFilterDelegate &f) noexcept {
    Zones filtered;
    for (auto &zone : areas) {
        if (f.filter(zone)) {
            filtered.emplace_back(zone);
        }
    }

    /* Copy elision */
    return filtered;
}

ConstZones Scene::zones(const ZoneFilterDelegate &f) const noexcept {
    ConstZones filtered;
    for (auto const&zone : areas) {
        if (f.filter(zone)) {
            filtered.emplace_back(zone);
        }
    }

    /* Copy elision */
    return filtered;
}

Zones Scene::zones(const ZoneFilter &filter) noexcept {
    Zones filtered;
    for (auto &zone : areas) {
        if (filter(zone)) {
            filtered.emplace_back(zone);
        }
    }

    /* Copy elision */
    return filtered;
}

ConstZones Scene::zones(const ZoneFilter &filter) const noexcept {
    ConstZones filtered;
    for (auto const&zone : areas) {
        if (filter(zone)) {
            filtered.emplace_back(zone);
        }
    }

    /* Copy elision */
    return filtered;
}

std::list<Zone> Scene::extract(const ZoneFilterDelegate &f) noexcept {
    std::list<Zone> exfiltered;

    for (auto it = areas.begin(); it != areas.end(); ) {
        auto n = std::next(it, 1);
        if (f.filter(*it)) {
            exfiltered.splice(exfiltered.end(), areas, it);
        }
        it = n;
    }

    /* Copy elision */
    return exfiltered;
}

std::list<Zone> Scene::extract(const ZoneFilter &filter) noexcept {
    std::list<Zone> exfiltered;

    for (auto it = areas.begin(); it != areas.end();) {
        auto n = std::next(it, 1);
        if (filter(*it)) {
            exfiltered.splice(exfiltered.end(), areas, it);
        } 
        it = n;
    }

    /* Copy elision */
    return exfiltered;
}

Scene Scene::remember() const noexcept {
    Scene copy;
    
    /* This is a copy of the scene to be used carefully */
    copy.view  = view;
    copy.areas = areas;

    return copy;
}

}  // namespace VPP
