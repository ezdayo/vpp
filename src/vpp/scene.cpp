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

#include "vpp/scene.hpp"

#include "vpp/log.hpp"

namespace VPP {

Scene::Scene() noexcept : view(), areas(), ts(0) { }

Zone &Scene::mark(Zone zone) noexcept {
    static uint64_t next_uuid = 0;
    static Zone     invalid;
 
    /* Crop the zone to prevent any issue and discard outside zones */
    static_cast<cv::Rect &>(zone) = zone & view.frame(); 
    if ((zone.width <= 0 ) || (zone.height <= 0)) {
        invalid = std::move(zone.copy(Zone::Copy::BBoxOnly));
        invalid.invalidate();
        return invalid;
    }

    zone.uuid = ++next_uuid;
    /* Update the zone state contents */
    zone.deproject(view);
    zone.tag++;

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

std::vector<Zone> Scene::extract(const ZoneFilterDelegate &f) noexcept {
    std::vector<Zone> exfiltered;

    for (auto it = areas.begin(); it != areas.end(); ) {
        if (f.filter(*it)) {
            exfiltered.emplace_back(std::move(*it));
            it = areas.erase(it);
        } else {
            ++it;
        }
    }

    /* Copy elision */
    return exfiltered;
}

std::vector<Zone> Scene::extract(const ZoneFilter &filter) noexcept {
    std::vector<Zone> exfiltered;

    for (auto it = areas.begin(); it != areas.end(); ) {
        if (filter(*it)) {
            exfiltered.emplace_back(std::move(*it));
            it = areas.erase(it);
        } else {
            ++it;
        }
    }

    /* Copy elision */
    return exfiltered;
}

void Scene::update(Scene &other) noexcept {
    for (auto it = other.areas.begin(); it != other.areas.end(); ) {
        if (it->valid()) {
            /* Crop the zone to prevent any issue and discard outside zones */
            static_cast<cv::Rect &>(*it) = (*it) & view.frame(); 
            if ((it->width > 0 ) && (it->height > 0)) {
                areas.emplace_back(std::move(*it));
            }
        }
        it = other.areas.erase(it);
    }
}

Scene Scene::remember() const noexcept {
    Scene copy;
    
    /* This is a copy of the scene to be used carefully */
    copy.view  = view;
    copy.areas = areas;
    copy.ts    = ts;

    return copy;
}

}  // namespace VPP
