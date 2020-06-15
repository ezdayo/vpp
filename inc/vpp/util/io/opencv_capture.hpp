/**
 *
 * @file      vpp/util/io/opencv_capture.hpp
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

#include <opencv2/core/core.hpp>
#include <opencv2/videoio.hpp>
#include <string>

#include "vpp/util/io/input.hpp"

namespace Util {
namespace IO {

class OCVCapture : public Input {
    public:
        OCVCapture() noexcept;
        virtual ~OCVCapture() noexcept;
        
        OCVCapture(const OCVCapture& other) = delete;
        OCVCapture(OCVCapture&& other) = delete;
        OCVCapture& operator=(const OCVCapture& other) = delete;
        OCVCapture& operator=(OCVCapture&& other) = delete;

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

} // namespace IO
} // namespace Util
