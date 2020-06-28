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

static Zone invalid;

Zone::Zone(BBox bbox, std::vector<Prediction> preds) noexcept
    : BBox(std::move(bbox)), state(), contour(), description(), marked(0) {
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

Zone &Zone::update(Zone &older) noexcept {
    ASSERT(valid(),
           "Zone::update(older) : Impossible to update an invalid zone.");
    ASSERT(older.valid(),
           "Zone::update(older) : Impossible to update a zone with an invalid "
           "one.");

    uuid    = older.uuid;
    marked += older.marked;

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
