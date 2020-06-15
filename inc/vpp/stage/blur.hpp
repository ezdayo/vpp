/**
 *
 * @file      vpp/stage/blur.hpp
 *
 * @brief     This is the VPP blur handling stage definition
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

#include "vpp/engine/blur.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Stage {

class Blur : public Stage::ForScene {
    public:
        Blur() noexcept;
        ~Blur() noexcept = default;

        VPP::Engine::Blur::Skipping skipping;
};

}  // namespace Stage
}  // namespace VPP
