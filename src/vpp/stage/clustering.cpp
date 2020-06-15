/**
 *
 * @file      vpp/stage/clustering.cpp
 *
 * @brief     This is the VPP clustering stage implementation
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

#include "vpp/stage/clustering.hpp"

namespace VPP {
namespace Stage {

Clustering::Clustering() noexcept : ForScene(true), basic() {
    use("basic", basic);
}

}  // namespace Stage
}  // namespace VPP
