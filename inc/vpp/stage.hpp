/**
 *
 * @file      vpp/stage.hpp
 *
 * @brief     This is the VPP pipeline stages description file
 *
 * @details   This is the definition of the various kinds of stages that can be
 *            used in the VPP. It is nothing but specialisations of the core
 *            stage templates.
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

#include "vpp/core/stage.hpp"
#include "vpp/scene.hpp"

namespace VPP {
namespace Stage {

/* Describing a stage for handling a full scene */
using ForScene = Core::Stage<>;

/* Describing a stage for handling a single zone in a scene */
using ForZone = Core::Stage<Zone>;

/* Describing a stage for handling multiple zones in a scene */
using ForZones = Core::Stage<Zones>;

}  // namespace Stage
}  // namespace VPP
