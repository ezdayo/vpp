/**
 *
 * @file      vpp/zone.cpp
 *
 * @brief     This is the VPP zone implementation file
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

#include "vpp/log.hpp"
#include "vpp/zone.hpp"

namespace VPP {

void Zone::Copy::UUID(Zone& out, const Zone &in) noexcept {
    out.uuid        = in.uuid;
    out.state       = in.state;
}

void Zone::Copy::Geometries(Zone& out, const Zone &in) noexcept {
    out.uuid        = in.uuid;
    out.state       = in.state;
}

void Zone::Copy::All(Zone& out, const Zone &in) noexcept {
    out.uuid        = in.uuid;
    out.state       = in.state;
    out.contour     = in.contour;
    out.predictions = in.predictions;
    out.description = in.description;
    out.tag      = in.tag;
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

void Zone::project(const View &view) noexcept {
    /* Update the zone from the state */
    auto centre   = state.centre;
    auto size     = state.size;
    cv::Point3f c = centre;
    cv::Point3f s(size.x/2, size.y/2, 0);
            
    auto tl       = view.depth.project(c-s);
    auto br       = view.depth.project(c+s);
    auto geom     = br - tl;

    x             = tl.x;
    y             = tl.y;
    width         = geom.x;
    height        = geom.y;
}

void Zone::deproject(const View &view) noexcept {
    /* Update the state from the zone */
    auto z = view.depth.at((cv::Rect::tl()+cv::Rect::br())/2);

    auto tl = view.depth.deproject(cv::Rect::tl(), z);
    auto br = view.depth.deproject(cv::Rect::br(), z);
    auto sz = br-tl;

    state.centre = (tl+br)/2;
    state.size.x = sz.x;
    state.size.y = sz.y;
}

Zone &Zone::update(Zone &older) noexcept {
    ASSERT(valid(),
           "Zone::update(older) : Impossible to update an invalid zone.");
    ASSERT(older.valid(),
           "Zone::update(older) : Impossible to update a zone with an invalid "
           "one.");

    uuid = older.uuid;
    tag += older.tag;

    predict(std::move(older.predictions));

    older.invalidate();

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

}  // namespace VPP
