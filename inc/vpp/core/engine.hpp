/**
 *
 * @file      vpp/core/engine.hpp
 *
 * @brief     This is the VPP core engine description file
 *
 * @details   This file describes the general structure of an engine, i.e. the
 *            entity responsible of processing a scene. 
 *            It can process a scene in different manners, either synchronously 
 *            or asynchronously by using core tasks.
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

#include <memory>

#include "customisation/entity.hpp"
#include "customisation/parameter.hpp"
#include "vpp/error.hpp"
#include "vpp/scene.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> class Engine : public Parametrisable {
    public:
        Engine() noexcept;

        /** Engines cannot be copied */
        Engine(const Engine& other) = delete;
        Engine(Engine&& other) = default;
        Engine& operator=(const Engine& other) = delete;
        Engine& operator=(Engine&& other) = default;
        virtual ~Engine() noexcept = default;

        /* Prepare the scene and its environment (for first stages) */
        virtual Error::Type prepare(Scene*& s, Z*&...z) noexcept;

        /* Processing a scene and its environment */
        virtual Error::Type process(Scene &s, Z&...z) noexcept;
};

}  // namespace Core
}  // namespace VPP
