/**
 *
 * @file      vpp/util/io/opencv_capture.cpp
 *
 * @brief     This is the OpenCV Capture Input class definition
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

#include "vpp/log.hpp"
#include "vpp/util/io/opencv_capture.hpp"

namespace Util {
namespace IO {
        
OCVCapture::OCVCapture() noexcept 
    : Input({ "ocv/file", "ocv/http", "ocv/https", "ocv/internal",
              "ocv/rtsp", "ocv/videoio"}), cap() {}

OCVCapture::~OCVCapture() noexcept = default;
        
int OCVCapture::open(const std::string &protocol, int id) noexcept {
    ASSERT((protocol == "ocv/internal"),
           "OCVCapture::open(): Unsupported protocol '%s'", protocol.c_str());
 
    if (protocol == "ocv/internal") {
        return open(id);
    } else {
        return -1;
    }
}

int OCVCapture::open(int id) noexcept {
    if (cap.open(id)) {
        return 0;
    } else {
        return -1;
    }
}

int OCVCapture::open(const std::string &protocol,
                     const std::string &source) noexcept {
    ASSERT(supports(protocol), "OCVCapture::open(): Unsupported protocol '%s'", 
           protocol.c_str());
    
    if ( (!source.empty()) && (protocol == "ocv/internal") &&
        (std::all_of(source.begin(), source.end(), ::isdigit)) ) {
        return open(protocol, atoi(source.c_str()));
    }

    if (supports(protocol)) {
        if (protocol != "ocv/file") {
            auto kind = protocol.substr(4, protocol.length()-4);
            std::string url(kind + "://" + source);
            return open(url);
        } else {
            return open(source);
        }
    } else {
        return -1;
    }
}

int OCVCapture::open(const std::string &url) noexcept {
    if (cap.open(url)) {
        return 0;
    } else {
        return -1;
    }
}

int OCVCapture::setup(const std::string &/*username*/,
                      const std::string &/*password*/) noexcept {
    /* Nothing to do, we are fine */
    return 0;
}

int OCVCapture::setup(int &width, int &height, int &rotation) noexcept {
    if (! cap.isOpened()) {
        return -1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

    width    = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    height   = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    rotation = 0;

    return 0;
}

int OCVCapture::read(cv::Mat &image) noexcept {
    if (! cap.isOpened()) {
        return -1;
    }

    cap >> image;

    return 0;
}

int OCVCapture::close() noexcept {
    if (cap.isOpened()) {
        cap.release();
    }

    return 0;
}

} // namespace IO
} // namespace Util