/**
 *
 * @file      vpp/engine.hpp
 *
 * @brief     This is the VPP engines description file
 *
 * @details   This is the definition of the various kinds of engines that can be
 *            used in the VPP. It is nothing but specialisations of the core
 *            engine templates.
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

#include "vpp/core/engine.hpp"
#include "vpp/scene.hpp"

namespace VPP {
namespace Engine {

/* Describing an engine for handling a full scene */
using ForScene = Core::Engine<>;

/* Describing an engine for handling a single zone in a scene */
using ForZone = Core::Engine<Zone>;

/* Describing an engine for handling multiple zones in a scene */
using ForZones = Core::Engine<Zones>;

}  // namespace Engine
}  // namespace VPP
