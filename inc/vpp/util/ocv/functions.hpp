/**
 *
 * @file      vpp/util/ocv/functions.hpp
 *
 * @brief     This is a collection of OpenCV helper functions
 *
 * @details   This is a collection of miscellaneous helper functions for OpenCV
 *            meant to factorise the code wherever possible.
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

#include <opencv2/core/types.hpp>

namespace Util {
namespace OCV {

/* This is the square euclidian norm for basic types */
template <typename T>
    static inline T square_norm(T x0, T y0, T x1, T y1) {
    return (x1-x0) * (x1-x0) + (y1-y0) * (y1 - y0);
}

/* This is the square euclidian norm for OpenCV points types */
template <typename P>
    static inline typename P::value_type square_norm(P p0, P p1) {
    auto d = p1-p0;
    return (d.x*d.x + d.y*d.y);
}

/* This is a measure of the affinity between two rectangles 
 * If the two rectangles overlap, then the affinity is the overlapping area; 
 * Otherwise this is the square minimal distance between the two rectangles
 */
template <typename R> typename R::value_type affinity(const R &a, const R &b);

}  // namespace OCV
}  // namespace Util
