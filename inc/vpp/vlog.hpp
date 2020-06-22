/**
 *
 * @file      vlog.hpp
 *
 * @brief     These are some C/C++ preprocessor MACRO for visual logging (VLOG)
 *            information
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

#include "vpp/config.hpp"

#pragma once

#ifndef VPP_HAS_OPENCV_GUI_SUPPORT

#define DISPLAY(tag, mat) ((void)0)
#define SHOW(tag, scene) ((void)0)

#else /* OPENCV GUI IS PRESENT */

#include <opencv2/highgui.hpp>

#define DISPLAY(tag, mat) { \
    cv::namedWindow(tag, cv::WINDOW_KEEPRATIO); \
    cv::imshow(tag, mat); \
    cv::waitKey(1); \
}

#define SHOW(tag, scene) DISPLAY(tag, scene.output())

#endif
