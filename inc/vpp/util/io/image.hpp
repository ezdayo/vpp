/**
 *
 * @file      vpp/util/io/image.hpp
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

#pragma once

#include <opencv2/core/core.hpp>
#include <string>
#include <vector>

#include "customisation/socket.hpp"
#include "vpp/util/io/input.hpp"

namespace Util {
namespace IO {

class Image : public Input {
    public:
        Image() noexcept;
        virtual ~Image() noexcept;
        
        Image(const Image& other) = delete;
        Image(Image&& other) = delete;
        Image& operator=(const Image& other) = delete;
        Image& operator=(Image&& other) = delete;

        virtual std::vector<std::string> modes() noexcept override;

        virtual int open(const std::string &protocol, int id) noexcept override;
        virtual int open(const std::string &protocol,
                         const std::string &source) noexcept override;

        virtual int setup(const std::string &username,
                          const std::string &password) noexcept override;
        virtual int setup(int &width, int &height, int &rotation) 
            noexcept override;

        virtual int read(cv::Mat &image) noexcept override;

        virtual int close() noexcept override;

    private:
        Customisation::Socket      socket;
        std::vector<unsigned char> data;
};

} // namespace IO
} // namespace Util
