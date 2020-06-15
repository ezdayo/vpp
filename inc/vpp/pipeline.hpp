/**
 *
 * @file      vpp/pipeline.hpp
 *
 * @brief     This is the VPP pipelines description file
 *
 * @details   This is the definition of the various kinds of pipelines that can
 *            be used in the VPP. It is nothing but specialisations of the core
 *            pipeline templates.
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

#include "vpp/core/pipeline.hpp"
#include "vpp/scene.hpp"

namespace VPP {
namespace Pipeline {

/* Describing a pipeline for handling a full scene */
using ForScene = Core::Pipeline<>;

/* Describing a pipeline for handling a single zone in a scene */
using ForZone = Core::Pipeline<Zone>;

/* Describing a pipeline for handling multiple zones in a scene */
using ForZones = Core::Pipeline<Zones>;

}  // namespace Pipeline
}  // namespace VPP
