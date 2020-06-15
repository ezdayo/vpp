/**
 *
 * @file      vpp/stage/input.cpp
 *
 * @brief     This is the VPP input stage implementation
 *
 *            This file is part of the VPP framework (see link).
 *
 * @details   This is always the first stage used in any VPP pipeline.
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#include "vpp/stage/input.hpp"

namespace VPP {
namespace Stage {

template <typename ...Z> Input<Z...>::Input() noexcept
    : Core::Stage<Z...>(false), bridge() {
    this->use("bridge", bridge);
}

Input<>::Input() noexcept : Core::Stage<>(false), bridge(), capture() {
    use("bridge",  bridge);
    use("capture", capture);
}

/* Create template implementations */
template class Input<Zone>;
template class Input<Zones>;

}  // namespace Stage
}  // namespace VPP
