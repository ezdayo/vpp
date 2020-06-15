/**
 *
 * @file      vpp/types.hpp
 *
 * @brief     This is a collection of basic types for the VPP
 *
 * @details   This is the definition of the various kinds of objects that can be
 *            used in the VPP.
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

#include "customisation/entity.hpp"
#include "customisation/parameter.hpp"

namespace VPP {

class Tile : public Customisation::Entity {
    public:
        Tile() noexcept : Entity("Tile"), width(0), height(0) {
            width.denominate("width")
                 .describe("The width in pixels")
                 .characterise(Customisation::Trait::CONFIGURABLE);
            expose(width);
            height.denominate("height")
                  .describe("The Height in pixels")
                  .characterise(Customisation::Trait::CONFIGURABLE);
            expose(height);
        }

        ~Tile() noexcept = default;

        operator cv::Size() const {
            return cv::Size(width, height);
        }

        operator cv::Rect() const {
            return cv::Rect(0, 0, width, height);
        }

        PARAMETER(Direct, WhiteListed, Immediate, int) width;
        PARAMETER(Direct, WhiteListed, Immediate, int) height;
};

using Size=Tile;
using Rect=Tile;

class Stride : public Customisation::Entity {
    public:
        Stride() noexcept : Entity("Stride"), x(0), y(0) {
            x.denominate("x")
             .describe("The offset in the x direction")
             .characterise(Customisation::Trait::CONFIGURABLE);
            expose(x);
            y.denominate("y")
             .describe("The offset in the y direction")
             .characterise(Customisation::Trait::CONFIGURABLE);
            expose(y);
        }

        ~Stride() noexcept = default;

        operator cv::Point() const {
            return cv::Point(x, y);
        }

        PARAMETER(Direct, WhiteListed, Immediate, int) x;
        PARAMETER(Direct, WhiteListed, Immediate, int) y;
};

using Offset=Stride;

}  // namespace VPP
