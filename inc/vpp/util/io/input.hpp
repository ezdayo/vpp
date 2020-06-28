/**
 *
 * @file      vpp/util/io/input.hpp
 *
 * @brief     This is the common Input interface class definition
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

#include <opencv2/core.hpp>
#include <set>
#include <string>
#include <vector>

#include "vpp/projection.hpp"
#include "vpp/image.hpp"

namespace Util {
namespace IO {

class Input {
    public:
        Input(std::set<std::string> protocols) noexcept;
        virtual ~Input() noexcept;
        
        Input(const Input& other) = delete;
        Input(Input&& other) = delete;
        Input& operator=(const Input& other) = delete;
        Input& operator=(Input&& other) = delete;

        std::set<std::string> protocols() const noexcept;
        bool supports(const std::string &protocol) const noexcept;
        virtual std::vector<std::string> sources() const noexcept;
        virtual std::vector<std::string> modes() noexcept;
 
        virtual int open(const std::string &protocol, int id) noexcept;

        virtual int open(const std::string &protocol,
                         const std::string &source) noexcept;

        virtual int setup(const std::string &username,
                          const std::string &password) noexcept;
        
        virtual int setup(int &width, int &height, int &rotation) noexcept;
        
        virtual int read(cv::Mat &image, VPP::Image::Mode &m) noexcept;

        virtual int close() noexcept;

        virtual VPP::Projecter *projecter() const noexcept;

    protected:
        std::set<std::string> valid;
};

} // namespace IO
} // namespace Util
