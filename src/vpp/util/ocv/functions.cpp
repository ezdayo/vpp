/**
 *
 * @file      vpp/util/ocv/functions.cpp
 *
 * @brief     This is the implementation of OpenCV helper functions
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


#include "vpp/util/ocv/functions.hpp"

namespace Util {
namespace OCV {

template <typename R> typename R::value_type affinity(const R &a, const R &b) {
    constexpr auto sq_norm = square_norm<typename R::value_type>;
    auto left   = ((b.x+b.width)  < a.x);
    auto right  = ((a.x+a.width)  < b.x);
    auto top    = ((b.y+b.height) < a.y);
    auto bottom = ((a.y+a.height) < b.y);

    if (top && left) {
        return -sq_norm(a.x, a.y+a.height, b.x+b.width, b.y);
    } else if (bottom && left) {
        return -sq_norm(a.x, a.y, b.x+b.width, b.y+b.height);
    } else if (bottom && right) {
        return -sq_norm(a.x+a.width, a.y, b.x, b.y+b.height);
    } else if (top && right) {
        return -sq_norm(a.x+a.width, a.y+b.height, b.x, b.y);
    } else if (left) {
        return -sq_norm(a.x, 0, b.x+b.width, 0);
    } else if (right) {
        return -sq_norm(a.x+a.width, 0, b.x, 0);
    } else if (bottom) {
        return -sq_norm(0, a.y, 0, b.y+b.height);
    } else if (top) {
        return -sq_norm(0, a.y+a.height, 0, b.y);
    } else {
        return (a & b).area();
    }
}

template int affinity(const cv::Rect2i &a, const cv::Rect2i &b); 
template float affinity(const cv::Rect2f &a, const cv::Rect2f &b); 
template double affinity(const cv::Rect2d &a, const cv::Rect2d &b); 

}  // namespace OCV
}  // namespace Util
