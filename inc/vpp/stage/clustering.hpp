/**
 *
 * @file      vpp/stage/clustering.hpp
 *
 * @brief     This is the VPP clustering stage definition
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

#include "vpp/engine/clustering.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {

class Clustering : public Stage::ForScene {
    public:
        Clustering() noexcept;
        ~Clustering() noexcept = default;

        VPP::Engine::Clustering basic;
};

}  // namespace Stage
}  // namespace VPP
