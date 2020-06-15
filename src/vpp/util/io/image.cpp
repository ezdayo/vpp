/**
 *
 * @file      vpp/util/io/image.cpp
 *
 * @brief     This is the image Input class definition
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

#include <opencv2/opencv.hpp>

#include "vpp/log.hpp"
#include "vpp/util/io/image.hpp"

namespace Util {
namespace IO {

Image::Image() noexcept
    : Input({ "image/http", "image/https", "image/file" }) {}
Image::~Image() noexcept = default;
 
std::vector<std::string> Image::modes() noexcept {
    return {};
}

int Image::open(const std::string &/*protocol*/, int /*id*/) noexcept {
    return -1;
}

int Image::open(const std::string &protocol,
                const std::string &source) noexcept {
    auto kind = protocol.substr(6, protocol.length()-6);
    ASSERT(supports(protocol), "Image::open(): unsupported protocol %s", 
           protocol.c_str());
    return socket.open(kind, source);
}

int Image::setup(const std::string &username,
                 const std::string &password) noexcept {
    return socket.setup(username, password);
}

int Image::setup(int &width, int &height, int &rotation) noexcept {
    cv::Mat test;

    auto error = read(test);
    LOGE("SETUP IMAGE");
    if (error) {
        LOGE("SETUP IMAGE: error %d", error);
        return error;
    }

    width    = test.cols;
    height   = test.rows;
    rotation = 0;

    return 0;
}

int Image::read(cv::Mat &image) noexcept {
    data.clear();

    auto error = socket.get(data);
    if (! error) {
        image = cv::imdecode(data, -1);
    }

    return error;
}

int Image::close() noexcept {
    return socket.close();
}

} // namespace IO
} // namespace Util
