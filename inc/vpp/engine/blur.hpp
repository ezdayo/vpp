/**
 *
 * @file      vpp/engine/blur.hpp
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

#pragma once

#include "customisation/parameter.hpp"
#include "vpp/error.hpp"
#include "vpp/scene.hpp"
#include "vpp/engine.hpp"
#include "vpp/task/blur.hpp"

namespace VPP {
namespace Engine {
namespace Blur {

class Skipping : public Engine::ForScene {
    public:
        Skipping() noexcept;
        ~Skipping() noexcept = default;

        Error::Type process(Scene &scene) noexcept override;

        Task::Blur::Skipping task;
};

}  // namespace Blur
}  // namespace Engine
}  // namespace VPP
