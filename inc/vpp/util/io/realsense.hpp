/**
 *
 * @file      vpp/util/io/realsense.hpp
 *
 * @brief     This is the Intel Realsense Input class definition
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
#ifndef VPP_HAS_REALSENSE_CAPTURE_SUPPORT
# error ERROR: VPP does not support the Realsense capture device!
#endif

#include <librealsense2/rs.hpp>

#include "vpp/util/io/input.hpp"

namespace Util {
namespace IO {

class Realsense : public Input {
public:
    class Core;
    
    Realsense() noexcept;
    virtual ~Realsense() noexcept;

    Realsense(const Realsense &other) = delete;
    Realsense(Realsense &&other) = delete;
    Realsense &operator=(const Realsense &other) = delete;
    Realsense &operator=(Realsense &&other) = delete;

    virtual std::vector<std::string> sources() const noexcept override;

    virtual std::vector<std::string> modes() noexcept override;

    virtual int open(const std::string &protocol, int id) noexcept override;

    virtual int open(const std::string &protocol,
                     const std::string &source) noexcept override;

    virtual int setup(const std::string &username,
                      const std::string &password) noexcept override;

    virtual int setup(int &width, int &height, int &rotation) noexcept override;

    virtual int read(cv::Mat &image, VPP::Image::Mode &m) noexcept override;

    virtual int close() noexcept override;

    virtual VPP::Projecter *projecter() const noexcept override;

private:
    int   stream;
    Core *core;
};

}  // namespace IO
}  // namespace Util
