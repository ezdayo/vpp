/**
 *
 * @file      vpp/util/ocv/capture.hpp
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

#pragma once

#include "vpp/config.hpp"
#ifndef VPP_HAS_OPENCV_VIDEO_IO_SUPPORT
# error ERROR: VPP does not support OpenCV video I/O!
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <string>

#include "vpp/util/io/input.hpp"

namespace Util {
namespace OCV {

class Capture : public Util::IO::Input {
    public:
        Capture() noexcept;
        virtual ~Capture() noexcept;
        
        Capture(const Capture& other) = delete;
        Capture(Capture&& other) = delete;
        Capture& operator=(const Capture& other) = delete;
        Capture& operator=(Capture&& other) = delete;

        virtual int open(const std::string &protocol, int id) noexcept override;
        int open(int id) noexcept;

        virtual int open(const std::string &protocol,
                         const std::string &source) noexcept override;
        int open(const std::string &url) noexcept;

        virtual int setup(const std::string &username,
                          const std::string &password) noexcept override;
        virtual int setup(int &width, int &height, int &rotation) 
            noexcept override;

        virtual int read(cv::Mat &image) noexcept override;

        virtual int close() noexcept override;

    private:
        cv::VideoCapture cap;
};

} // namespace OCV
} // namespace Util
