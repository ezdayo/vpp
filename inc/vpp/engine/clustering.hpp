/**
 *
 * @file      vpp/engine/clustering.hpp
 *
 * @brief     This is a generic clustering engine
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
#include "error.hpp"
#include "vpp/scene.hpp"
#include "vpp/engine.hpp"
#include "vpp/task/clustering.hpp"

namespace VPP {
namespace Engine {

class Clustering : public Engine::ForScene {
    public:
        Clustering() noexcept;
        ~Clustering() noexcept = default;

        Error::Type process(Scene &scene) noexcept override;

        Task::Clustering::DilateAndJoin dnj;
        Task::Clustering::Similarity    similarity;
};

}  // namespace Engine
}  // namespace VPP
