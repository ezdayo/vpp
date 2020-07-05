/**
 *
 * @file      vpp/task/edging.hpp
 *
 * @brief     This is a task for detecting the edges in a scene
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

#include <vector>

#include "vpp/error.hpp"
#include "vpp/task.hpp"
#include "vpp/scene.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace Task {

class Edging : public VPP::Task::Single<Edging, Scene&> {
    public:
        using Parent = VPP::Task::Single<Edging, Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::start;

        explicit Edging(const int mode) noexcept;
        ~Edging() noexcept = default;

        PARAMETER(Direct, Saturating, Immediate, int) input_scale;
        PARAMETER(Direct, Saturating, Immediate, std::vector<int>) blur_size;
        PARAMETER(Direct, Saturating, Immediate, int) min_area;
        PARAMETER(Direct, Saturating, Immediate, int) threshold_low;
        PARAMETER(Direct, Saturating, Immediate, int) threshold_high;
        PARAMETER(Direct, Saturating, Immediate, int) kernel_size;
        PARAMETER(Direct, Saturating, Immediate, int) levels;

        Error::Type process(Scene &scene) noexcept;
};

}  // namespace Task
}  // namespace VPP
