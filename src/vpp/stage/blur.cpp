/**
 *
 * @file      vpp/stage/blur.cpp
 *
 * @brief     This is the VPP blur handling stage implementation
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

#include "vpp/stage/blur.hpp"

namespace VPP {
namespace Stage {

Blur::Blur() noexcept : ForScene(true), skipping() {
    use("skipping", skipping);
}

}  // namespace Stage
}  // namespace VPP
