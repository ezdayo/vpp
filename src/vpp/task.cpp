/**
 *
 * @file      vpp/task.cpp
 *
 * @brief     This is the VPP tasks implementation
 *
 * @details   These are the base class of all VPP pipeline tasks.
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

#include "core/task.tpl.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Core {

/* Create core template implementations */
template class Task<>;
template class Task<Zone>;
template class Task<Zones>;

template class Tasks<Zone>;
template class Tasks<Zone, Zones>;
template class Tasks<cv::Rect, cv::Rect>;

}  // namespace Core

namespace Tasks {

ForScene::ForScene(const int mode) noexcept : Core::Tasks<Zone>(mode) {
}

Error::Type ForScene::start(Scene &s) noexcept {
    zones = s.zones();
    it    = zones.begin();

    return Core::Tasks<Zone>::start(s);
}

Error::Type ForScene::next(Scene &/*s*/, Zone &z) noexcept {
    if (it != zones.end()) {
        z = (*it).get();
        ++it;
        return Error::OK;
    }
    return Error::NOT_EXISTING;
}

ForZones::ForZones(const int mode) noexcept : Core::Tasks<Zone, Zones>(mode) {
}

Error::Type ForZones::start(Scene &s, Zones &zs) noexcept {
    it = zs.begin();

    return Core::Tasks<Zone, Zones>::start(s, zs);
}

Error::Type ForZones::next(Scene &/*s*/, Zone &z, Zones &zs) noexcept {
    if (it != zs.end()) {
        z = (*it).get();
        ++it;
        return Error::OK;
    }
    return Error::NOT_EXISTING;
}

Tiled::Tiled(const int mode) noexcept : Core::Tasks<cv::Rect, cv::Rect>(mode) {
    tile.denominate("tile")
        .describe("The tile geometry to use for the processing");
    expose(tile);

    /* Use default macro-block size for starting */
    tile.width  = 16;
    tile.height = 16;

    stride.denominate("stride")
          .describe("The stride to use for the processing");
    expose(stride);
    
    /* Use default macro-block stride for starting */
    stride.x = 16;
    stride.y = 16;
}

Error::Type Tiled::start(Scene &s, cv::Rect &frame) noexcept {
    it = cv::Rect(frame.tl(), static_cast<cv::Size>(tile));
    tiles_total = 0;
    return Core::Tasks<cv::Rect, cv::Rect>::start(s, frame);
}

Error::Type
    Tiled::next(Scene &/*s*/, cv::Rect &roi, cv::Rect &frame) noexcept {
    if (frame.contains(it.tl())) {
        roi = it;
        ++ tiles_total;
    
        /* Compute the next location */
        it.x += stride.x;
        if (!frame.contains(it.tl())) {
            it.x = frame.x;
            it.y += stride.y;
        }

        return Error::OK;
    }
    return Error::NOT_EXISTING;
}

}  // namespace Tasks
}  // namespace VPP
