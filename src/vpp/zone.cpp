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

void Zone::Copy::Geometry(Zone& out, const Zone &in) noexcept {
    out.state       = in.state;
}

void Zone::Copy::AllButContour(Zone& out, const Zone &in) noexcept {
    out.state       = in.state;
    out.predictions = in.predictions;
    out.description = in.description;
}

void Zone::Copy::All(Zone& out, const Zone &in) noexcept {
    out.state       = in.state;
    out.contour     = in.contour;
    out.predictions = in.predictions;
    out.description = in.description;
}

Zone &Zone::predict(Prediction pred, float recall_f) noexcept {
    if (predictions.empty()) {
        predictions.emplace_front(std::move(pred)); 
        context = predictions.front();
        return *this;
    } 
    
    std::list<Prediction> preds;
    preds.emplace_front(std::move(pred));
    return predict(std::move(preds), recall_f);
}

/* Add a forgetting factor ? */
Zone &Zone::predict(std::list<Prediction> preds, float recall_f) noexcept {
    if (! preds.empty()) {

        if (!predictions.empty()) {
            /* Create a map of previously existing predictions (there is only
             * one entry at most per GID */
            std::unordered_map<int32_t, Prediction *> existing;
            for (auto &p : predictions) {
                p.score *= recall_f;
                existing.emplace(p.gid(), &p);
            }

            /* Append new predictions or update the old one if the new one is
             * higher than the old one */
            for (auto &p : preds) {
                auto found = existing.find(p.gid());
                if (found != existing.end()) {
                    auto &o = *found->second;
                    if (p.score > o.score) {
                        o.score = p.score;
                    }
                } else {
                    predictions.push_back(p);
                }
            }
        } else {
            predictions = std::move(preds);
        }

        predictions.sort(std::greater<Prediction>());
        if ( (context.id < 0) && (!predictions.empty()) ) {
            context = predictions.front();
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

Zone &Zone::update(Zone &older, float recall_f) noexcept {
    ASSERT(valid(),
           "Zone::update(older) : Impossible to update an invalid zone.");
    ASSERT(older.valid(),
           "Zone::update(older) : Impossible to update a zone with an invalid "
           "one.");

    uuid = older.uuid;

    if ((contour.empty()) && (!older.contour.empty())) {
        contour = std::move(older.contour);
    }

    /* Apply the memory forget factor to the older predictions! */
    older.predict(predictions, recall_f);
    predictions = std::move(older.predictions);

    older.invalidate();

    return *this;
}
 
Zone &Zone::merge(const Zone &zone) noexcept {
    static_cast<cv::Rect&>(*this) = *this | zone;
    uuid   = 0; /* Needs a new UUID when marked on a scene! */
     
    if ((contour.empty()) && (!zone.contour.empty())) {
        contour = zone.contour;
    }
    predict(zone.predictions);
    if ((description.empty()) && (!zone.description.empty())) {
        description = zone.description;
    }

    return *this;
}

Zone Zone::merge(const Zones &zones) noexcept {
    Zone zone;

    for (auto &z : zones) {
        zone.merge(z.get());
    }
 
    return zone;
}

float Zone::recall = 1.0;

}  // namespace VPP
