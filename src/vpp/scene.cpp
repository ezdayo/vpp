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

static Zone invalid;

Zone::Zone(BBox bbox, std::vector<Prediction> preds) noexcept
    : BBox(std::move(bbox)), step(0) {
        predict(std::move(preds));
}

Zone &Zone::predict(Prediction pred) noexcept {
    predictions.emplace_back(std::move(pred));
    std::sort(predictions.begin(), predictions.end(),
              std::greater<Prediction>());
    
    if (context.id < 0) {
        context = predictions[0];
    }
    
    return *this;
}

Zone &Zone::predict(std::vector<Prediction> preds) noexcept {
    if (! preds.empty()) {
        std::move(preds.begin(), preds.end(), std::back_inserter(predictions));
        std::sort(predictions.begin(), predictions.end(),
                  std::greater<Prediction>());
        if (context.id < 0) {
            context = predictions[0];
        }
    }

    return *this;
}
 
bool Zone::predict(float dt) noexcept {
    tracked.predict(dt);
    
    return tracked.valid();
}

Zone &Zone::update(Zone &older) noexcept {
    older.tracked.correct(static_cast<Tracker::Measure>(tracked));
    tracked = std::move(older.tracked);

    predict(std::move(older.predictions));

    return *this;
}

Zone &Zone::merge(const Zone &zone) noexcept {
    auto box = *this | zone;
    x      = box.x;
    y      = box.y;
    width  = box.width;
    height = box.height;

    predict(zone.predictions);

    return *this;
}

Zone Zone::merge(const Zones &zones) noexcept {
    std::vector<Prediction> preds;
    cv::Rect                area;

    for (auto &z : zones) {
        area = area | z.get();
        std::move(z.get().predictions.begin(), z.get().predictions.end(),
                  std::back_inserter(preds));
    }
 
    Zone zone(area, preds);
    zone.describe(zones.front().get().description);

    return zone;
}

float Zone::similarity(const Zone &other) const noexcept {
    for (auto const &p : predictions) {
        if (p.score < 0.1) {
            break;
        }

        for (auto const &o : other.predictions) {
            if (o.score < 0.1) {
                break;
            }

            if (p.gid() == o.gid()) {
                return static_cast<float>((*this & other).area())/
                       static_cast<float>((*this | other).area());
            }
        }
    }

    return 0.0;
}

class NoProjection : public Util::OCV::ProjectionDelegate {
public:
    NoProjection() = default;
    virtual ~NoProjection() = default;

    cv::Point project(const cv::Point3f &p) const noexcept override;
    cv::Point3f deproject(const cv::Point &p, 
                          uint16_t z) const noexcept override;
};

static NoProjection no_projection;

cv::Point NoProjection::project(const cv::Point3f &p) 
    const noexcept {
        return cv::Point(p.x, p.y);
}

cv::Point3f NoProjection::deproject(const cv::Point &p,
                                            uint16_t /*z*/) const noexcept {
        return cv::Point3f(p.x, p.y, 0);
}

Scene::Scene() noexcept
    : areas(), ts(0), fimage(), iimage(), oimage(), fdmap(), dmap(), 
      projection(nullptr) {}

void Scene::use(cv::Mat image) noexcept {
    use(std::move(image), no_projection);
}

void Scene::use(cv::Mat image, 
                const Util::OCV::ProjectionDelegate &pd) noexcept {
    auto is_color = (image.channels() == 3);
    auto is_depth = (image.channels() == 1);
    if ( ((is_color)  && (!iimage.empty())) ||
         ((is_depth)  && (!dmap.empty()))  ||
         ((!is_color) && (!is_depth)) || 
         (image.empty()) ) {
        return;
    }

    /* Store the relevant information */
    if (is_color) {
        fimage = std::move(cv::Rect(0, 0, image.cols, image.rows));
        iimage = std::move(image);
    } else {
        fdmap = std::move(cv::Rect(0, 0, image.cols, image.rows));
        dmap  = std::move(image);
    }

    /* Update the timestamp if none is set */
    if (ts == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto now_ms = 
            std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        ts = now_ms.time_since_epoch().count();
    }

    /* Update the projection delegate if not already set */
    if ( (&pd != &no_projection) && 
         (projection != &no_projection) ) {
        projection = &pd;
    }
}

cv::Point3f Scene::point_at(const cv::Point &pix, uint16_t z) const noexcept {
    if (fdmap.contains(pix) && (z > 0)) {
        return projection->deproject(pix, z);
    }

    return cv::Point3f(pix.x, pix.y, 0);
}

uint16_t Scene::depth_at(const cv::Point &pix) const noexcept {
    uint16_t z = 0;
    if (fdmap.contains(pix)) {
        /* If the centre is a valid depth point, then use it */
       /* z = dmap.at<uint16_t>(pix);
        if ((z > 0) && (z < std::numeric_limits<uint16_t>::max())) {
            return z;
        }*/

        /* Otherwise get the average around the required pixel */
        for (int s=8; s < 16; ++s) {
            cv::Rect r(pix.x-s, pix.y-s, 2*s+1, 2*s+1);
            r = r & fdmap;
            auto zd = dmap(r);
            cv::Mat zm = (zd > 0); 
            /*double minz[1], maxz[1];
            cv::minMaxLoc(zd, minz, maxz, NULL, NULL, zm);
            z = minz[0];*/
            z = static_cast<uint16_t>(cv::mean(zd, zm)[0]);
            if (z > 0) {
                return z;
            }
        }

        z = 0;
    }

    return z;
}

cv::Point Scene::pixel_at(const cv::Point3f &p) const noexcept {
    return projection->project(p);
}

const cv::Mat &Scene::output() const noexcept {
    return (oimage.empty()) ? iimage : oimage;
}

cv::Mat &Scene::drawable() noexcept {
    if (oimage.empty()) {
        flush();
    }

    return oimage;
}

void Scene::flush() noexcept {
    iimage.copyTo(oimage);
}

Zone &Scene::mark(Zone zone) noexcept {
    /* Crop the zone to prevent any issue and discard outside zones */
    static_cast<cv::Rect &>(zone) = zone & fimage; 
    if ((zone.width <= 0 ) || (zone.height <= 0)) {
        return invalid;
    }

    /* Perform real-size measurement (if not already set) */
    if (!zone.tracked.valid()) {
        auto z = depth_at((zone.tl()+zone.br())/2);
        if (z <= 0) {
            z = 10000;
        }
        auto tl = point_at(zone.tl(), z);
        auto br = point_at(zone.br(), z);
        auto sz = br-tl;

        Tracker::Measure measure;
        measure.centre = (tl+br)/2;
        measure.size.x = sz.x;
        measure.size.y = sz.y;
        zone.tracked   = measure;
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

Scene Scene::remember() const noexcept {
    Scene copy;
    
    /* Deep copy of the vector content */
    copy.areas      = areas;
    copy.ts         = ts;
    copy.projection = projection;

    return copy;
}

void Scene::predict(const Scene &newer) noexcept {
    /* Compute the delta time in ms */
    auto dt_ms = newer.ts - ts;
    float dt = static_cast<float>(dt_ms) / 1000.0f;

    /* Predict the updated locations of all zones */
    for (auto it = areas.begin(); it != areas.end(); ) {
        if (it->predict(dt)) {

            /* Update the zone spatial contents */
            auto centre   = it->tracked.centre;
            auto size     = it->tracked.size;
            cv::Point3f c = centre;
            cv::Point3f s(size.x/2, size.y/2, 0);

            auto tl       = pixel_at(c-s);
            auto br       = pixel_at(c+s);
            auto geom     = br - tl;

            it->x         = tl.x;
            it->y         = tl.y;
            it->width     = geom.x;
            it->height    = geom.y;
            
            ++it;
        } else {
            it = areas.erase(it);
        }
    }
}

Scene Scene::update(Scene &older) noexcept {

    /* Predict the older scene from this one */
    older.predict(*this);

    /* Map old to new zones (if possible) */
    /*for (auto &z : areas) {
        float max_s  = 0;
        auto  max_it = older.areas.begin();
        for (auto it = older.areas.begin(); it != older.areas.end(); ++it) {
            auto s = z.similarity(*it);
            if (s > max_s) {
                max_it = it;
                max_s = s;
            }
        }

        if (max_s > 0.5) {
            z.update(*max_it);
            older.areas.erase(max_it);
        }
    }*/

    for (auto &z : areas) {
        float max_s = 0;
        auto  max_it = older.areas.begin();
        for (auto it = older.areas.begin(); it != older.areas.end(); ++it) {
            float s = 0;
            if (z.context.gid() == it->context.gid()) {
                s = it->tracked.similarity(z.tracked);
            }
            if (s > max_s) {
                max_it = it;
                max_s = s;
            }
        }

        if (max_s > 0) {
            z.update(*max_it);
            older.areas.erase(max_it);
        }
    }

    /* Append the remaining "old" zones */
    for (auto it = older.areas.begin(); it != older.areas.end(); ) {
        /* Crop the zone to prevent any issue and discard outside zones */
        static_cast<cv::Rect &>(*it) = (*it) & fimage; 
        if ((it->width > 0 ) && (it->height > 0)) {
            areas.emplace_back(std::move(*it));
        }
        it = older.areas.erase(it);
    }

    /* Return a copy for reference */
    return remember();
}

}  // namespace VPP
