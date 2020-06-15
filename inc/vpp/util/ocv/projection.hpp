/**
 *
 * @file      vpp/util/ocv/projection.hpp
 *
 * @brief     This is a 3D to 2D (and vice-versa) projection delegate definition
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

namespace Util {
namespace OCV {

class ProjectionDelegate {
    public:
        ProjectionDelegate() noexcept = default;
        virtual ~ProjectionDelegate() noexcept = default;

        virtual cv::Point project(const cv::Point3f &p) const noexcept = 0;
        virtual cv::Point3f deproject(const cv::Point &p,
                                      uint16_t z) const noexcept = 0;
};

}  // namespace OCV
}  // namespace Util
