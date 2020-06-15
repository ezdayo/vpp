/**
 *
 * @file      vpp/engine.cpp
 *
 * @brief     This is the VPP engines implementation
 *
 * @details   These are the base classes of all VPP pipeline engines.
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

#include "core/engine.tpl.hpp"
#include "vpp/engine.hpp"

namespace VPP {
namespace Core {

/* Create template implementations */
template class Engine<>;
template class Engine<Zone>;
template class Engine<Zones>;

}  // namespace Core
}  // namespace VPP
