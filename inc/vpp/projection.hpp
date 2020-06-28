/**
 *
 * @file      vpp/projection.hpp
 *
 * @brief     This is a 3D to 2D (and vice-versa) projection definition
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

#include <opencv2/core/core.hpp>

namespace VPP {

class Projecter {
    public:
        Projecter() noexcept = default;
        virtual ~Projecter() noexcept = default;

        virtual cv::Point project(const cv::Point3f &p) const noexcept = 0;
        virtual cv::Point3f deproject(const cv::Point &p,
                                      float z) const noexcept = 0;
        float   zscale;
};

using ProjectionDelegate = Projecter;

}  // namespace VPP
