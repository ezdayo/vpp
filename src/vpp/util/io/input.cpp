/**
 *
 * @file      vpp/util/io/input.cpp
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

#include <utility>

#include "vpp/log.hpp"
#include "vpp/util/io/input.hpp"

namespace Util {
namespace IO {

Input::Input(std::set<std::string> protocols) noexcept
    : valid(std::move(protocols)) {}
 
Input::~Input() noexcept = default;
 
bool Input::supports(const std::string &protocol) const noexcept {
    return valid.find(protocol) != valid.end();
}

std::set<std::string> Input::protocols() const noexcept {
    return valid;
}

std::vector<std::string> Input::sources() const noexcept {
    return {};
}

static std::vector<std::pair<int, int>> test_modes = {
    /* 4:3 */ 
    {  640,  480 }, {  768,  576 }, {  800,  600 }, { 1024,  768 },
    { 1152,  864 }, { 1280,  960 }, { 1400, 1050 }, { 1440, 1080 }, 
    { 1600, 1200 }, { 1920, 1440 }, { 2048, 1536 },  
    /* 8:5 */ 
    {  768,  480 }, { 1152,  720 }, { 1280,  800 }, { 1440,  900 },
    { 1680, 1050 }, { 1920, 1200 }, { 2304, 1440 }, { 2560, 1600 },
    /* 16:9 */ 
    {  854,  480 }, { 1024,  576 }, { 1280,  720 }, { 1366,  768 }, 
    { 1600,  900 }, { 1920, 1080 }, { 2560, 1440 },
};

std::vector<std::string> Input::modes() noexcept {
    std::vector<std::string> ok;
    int w, h, r;
    r = 0;

    for (auto &m : test_modes) {
        w = m.first;
        h = m.second;

        if ((! setup(w, h, r)) && (w == m.first) && (h == m.second)) {
            ok.emplace_back(std::to_string(w)+"x"+std::to_string(h));
        }
    }

    return ok;
}

int Input::open(const std::string &protocol, int id) noexcept {
    LOGE("Input::open(const string &, int) not defined \n"
         "in child classes");
    LOGE("-> called as open(\"%s\", %d", protocol.c_str(), id);
    assert(false);

    return -1;
}

int Input::open(const std::string &protocol,
                const std::string &source) noexcept {
    LOGE("Input::open(const string &, const string &) "
         "not defined in child classes");
    LOGE("-> called as open(\"%s\", \"%s\")", protocol.c_str(), 
         source.c_str());
    assert(false);

    return -1;
}

int Input::setup(const std::string &username,
                 const std::string &password) noexcept {
    LOGE("Input::setup(const string &, const string &) \n"
         "not defined in child classes");
    LOGE("-> called as setup(\"%s\", \"%s\")", username.c_str(),
         password.c_str());
    assert(false);

    return -1;
}

int Input::setup(int &width, int &height, int &rotation) noexcept {
    LOGE("Input::setup(int &, int &, int &) not defined in child class");
    LOGE("-> called as setup(%d, %d, %d", width, height, rotation);
    assert(false);

    return -1;
}
 
int Input::read(cv::Mat & /*image*/, VPP::Image::Mode & /*mode*/) noexcept {
    LOGE("Input::read(cv::Mat &) not defined in child class");
    assert(false);

    return -1;
}

int Input::close() noexcept {
    LOGE("Input::close() not defined in child class");
    assert(false);

    return -1;
}

VPP::Projecter *Input::projecter() const noexcept {
    return nullptr;
}

} // namespace IO
} // namespace Util
