/**
 *
 * @file      vpp/engine/blur.cpp
 *
 * @brief     These are various engines to manage blur with images
 *
 * @details   This is a collection of engines for managing images that are
 *            blurred
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

#include "vpp/log.hpp"
#include "vpp/engine/blur.hpp"

namespace VPP {
namespace Engine {
namespace Blur {

Skipping::Skipping() noexcept : task(Tasks::Tiled::Mode::Async*8) {
    
    task.tile.width  = 16;
    task.tile.height = 16;
    task.stride.x    = 16;
    task.stride.y    = 16;
    task.sharpness   = 300;
    task.coverage    = 0.01;

    task.denominate("process");
    expose(task);
}

Error::Type Skipping::process(Scene &scene) noexcept {
    cv::Rect frame(scene.frame());
    task.start(scene, frame);
    auto e = task.wait();

    return e;
}

}  // namespace Blur
}  // namespace Engine
}  // namespace VPP
