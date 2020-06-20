/**
 *
 * @file      vpp/task/blur.hpp
 *
 * @brief     These are various tasks to manage blur with images
 *
 * @details   This is a collection of tasks for managing images that are blurred
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

#include "customisation/parameter.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Blur {

class Skipping : public Tasks::Tiled {
    public:
        explicit Skipping(const int mode) noexcept;
        virtual ~Skipping() noexcept = default;

        /* Sharpness threshold, the minimum sharpness measure to consider a tile
         * as not being blurred */
        PARAMETER(Direct, Bounded, Immediate, float) sharpness;

        /* Coverage threshold, the minimal ratio of non blurred tiles not to
         * skip the scene */
        PARAMETER(Direct, Bounded, Immediate, float) coverage;

        Error::Type start(Scene &s, cv::Rect &frame) noexcept;
        Error::Type wait() noexcept;

    protected:
        virtual Error::Type process(Scene &s, cv::Rect &r) noexcept override;

    private:
        int tiles_valid;
};

}  // namespace Blur
}  // namespace Task
}  // namespace VPP
