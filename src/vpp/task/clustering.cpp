/**
 *
 * @file      vpp/task/clustering.cpp
 *
 * @brief     These are all the various VPP clustering tasks
 *
 * @details   This is a collection of tasks for clustering zones along different
 *            algorithms
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

#include <climits>
#include <opencv2/objdetect.hpp>

#include "vpp/log.hpp"
#include "vpp/task/clustering.hpp"
#include "vpp/util/ocv/functions.hpp"

namespace VPP {
namespace Task {
namespace Clustering {

DilateAndJoin::DilateAndJoin(const int mode) noexcept 
    : ForScene(mode), filter([](const Zone &){ return true; }) {
    ratio.denominate("ratio")
         .describe("Dilatation ratio to apply prior to joining overlapping "
                   "zones. It is a dilatation when > 1 and a contraction when "
                   "< 1")
         .characterise(Customisation::Trait::SETTABLE);
    ratio.range(-0.99, 100); 
    ratio = 0;
    expose(ratio);

    cross.denominate("cross")
         .describe("Flag to apply cross dilatation, when true, or a standard "
                   "dilatation when false. A cross dilatation is when dilating "
                   "witdh by a ratio of the height and the height by a ratio "
                   "of the width")
         .characterise(Customisation::Trait::SETTABLE);
    cross.use(Customisation::Translator::BoolFormat::NO_YES);
    cross = false;
    expose(cross);
}

Error::Type DilateAndJoin::process(Scene &scn) noexcept {
    /* Matching zones are removed from the scene zones */
    auto to_cluster(scn.extract(filter));
    std::vector<Zone> clusters;
                    
    for (auto const &z : to_cluster) {
        int dx, dy;
        if (cross) {
            dx = static_cast<int>(z.height*ratio);
            dy = static_cast<int>(z.width*ratio);

            /* If a contraction is making the zone disappear, then do not keep
             * the zone! */
            if ( (z.width + dx <= 0) || (z.height + dy <= 0) ) {
                continue;
            }
        } else {
            dx = static_cast<int>(z.width*ratio);
            dy = static_cast<int>(z.height*ratio);
        }

        /* Apply the transform to the zone */
        Zone zone(z);
        zone.x      -= dx/2;
        zone.width  += dx;
        zone.y      -= dy/2;
        zone.height += dy;

        clusters.emplace_back(zone);
    }

    /* Join overlapping clusters */
    bool joined;
    do {
        joined = false;
        for (auto cluster = clusters.begin(); cluster != clusters.end(); ) {
            for (auto other = cluster+1; other != clusters.end(); ) {
                if ((*cluster & *other).area() > 0) {
                    (*cluster).merge(*other);
                    clusters.erase(other);
                    other = cluster + 1;
                    joined = true;
                } else {
                    ++other;
                }
            }
            ++cluster;
        }
    } while (joined);
                
    /* Let's put back the clustered zones in the scene */
    for (auto cluster : clusters) {
        scn.mark(std::move(cluster));
    }

    return Error::OK;
}

Similarity::Similarity(const int mode) noexcept 
    : ForScene(mode), filter([](const Zone &){ return true; }) {
    threshold.denominate("threshold")
             .describe("Threshold for the similarity clustering. The smaller "
                       "the threshold, the pickier the clustering is")
             .characterise(Customisation::Trait::SETTABLE);
    threshold = 1.0;
    expose(threshold);
}

Error::Type Similarity::process(Scene &scn) noexcept {
    constexpr auto rect_affinity = Util::OCV::affinity<cv::Rect>;

    /* Matching zones are removed from the scene zones */
    auto to_cluster(scn.extract(filter));
    std::vector<cv::Rect> areas;

    /* Put every zone twice so that at least two can be grouped in a cluster */
    for (auto const r : to_cluster) {
        areas.push_back(r);
        areas.push_back(r);
    }

    /* Group rectangles through a similarity criteria */
    cv::groupRectangles(areas, 1, threshold);
    if (areas.size() == 0) {
        for (auto z: to_cluster) {
            scn.mark(std::move(z));
        }

        return Error::OK;
    }

    std::vector<Zone> clusters(areas.begin(), areas.end());
    
    /* Map each zone with the cluster that it has most affinity with */
    std::vector<Zones> cluster_map(clusters.size(), Zones());
    for (auto &zone : to_cluster) {
        int best_affinity = INT_MIN;
        auto map_it       = cluster_map.begin();
        auto map_it_best  = map_it;
        for (auto const &cluster : clusters) {
            auto affinity = rect_affinity(zone, cluster);
            if (affinity > best_affinity) {
                best_affinity = affinity;
                map_it_best   = map_it;
            }
            ++ map_it;
        }
        (*map_it_best).emplace_back(zone);
    }

    /* Append the cluster at the end of the cluster map */
    auto map_it = cluster_map.begin();
    for (auto &cluster : clusters) {
         (*map_it).emplace_back(cluster);
         ++map_it;
    }

    /* Going through the original list to keep the initial ordering */
    for (auto const &zone : to_cluster) {
        for (auto const &mapped : cluster_map) {
            if (&zone == &mapped.front().get()) {
                scn.mark(Zone::merge(mapped));
            }
        }
    }

    return Error::OK;
}

}  // namespace Clustering
}  // namespace Task
}  // namespace VPP
