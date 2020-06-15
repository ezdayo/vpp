/**
 *
 * @file      vpp/stage.cpp
 *
 * @brief     This is the VPP pipeline stages implementation
 *
 * @details   This is the base classes of all VPP pipeline stages.
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

#include "core/stage.tpl.hpp"
#include "vpp/stage.hpp"

namespace VPP {
namespace Core {

/* Create template implementations */
template class Stage<>;
template class Stage<Zone>;
template class Stage<Zones>;

}  // namespace Core
}  // namespace VPP
