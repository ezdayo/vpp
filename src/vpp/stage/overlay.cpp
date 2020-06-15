/**
 *
 * @file      vpp/stage/overlay.cpp
 *
 * @brief     This is the VPP overlay handling stage implementation
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

#include "vpp/stage/overlay.hpp"

namespace VPP {
namespace Stage {
namespace Overlay {

template <typename ...Z> 
Core<Z...>::Core() noexcept : VPP::Core::Stage<Z...>(true), ocv() {
    VPP::Core::Stage<Z...>::use("ocv", ocv);
}

/* Create template implementations */
template class Core<>;
template class Core<Zone>;
template class Core<Zones>;

}  // namespace Overlay
}  // namespace Stage
}  // namespace VPP
