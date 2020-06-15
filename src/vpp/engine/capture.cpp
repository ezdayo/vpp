/**
 *
 * @file      vpp/engine/capture.cpp
 *
 * @brief     This is the VPP capture engine implementation
 *
 * @details   This is an engine wrapper around all available VPP video inputs.
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

#include <sstream>

#include "vpp/log.hpp"
#include "vpp/engine/capture.hpp"
#ifdef __ANDROID__
#include "vpp/util/io/android_camera.hpp"
#endif
#include "vpp/util/io/image.hpp"
#include "vpp/util/io/realsense.hpp"
#include "vpp/util/io/opencv_capture.hpp"

namespace VPP {
namespace Engine {

/* Helper function to extract the mode from a string */
static bool string2mode(const std::string &m, int &w, int &h, int &r) noexcept {
    r = 0;

    std::istringstream stream(m);
    stream >> w;
    stream.ignore(1, 'x');
    stream >> h;
    if (!stream.eof()) {
        stream.ignore(1, '@');
        stream >> r;
    }
 
    return (stream.eof() && (!stream.fail()));
}

Capture::Capture() noexcept : sources(), current(nullptr), next(nullptr) {
    /* When seeking a source, seek first for native cameras, then WIFI P2P and
     * fall back to OpenCV VideoCapture in last resort */
#ifdef __ANDROID__
    sources.emplace_back(std::unique_ptr<Util::IO::Input>(std::move(new 
                                                Util::IO::AndroidCamera())));
#endif
    sources.emplace_back(std::unique_ptr<Util::IO::Input>(std::move(new 
                                                    Util::IO::Realsense())));
    sources.emplace_back(std::unique_ptr<Util::IO::Input>(std::move(new 
                                                    Util::IO::Image())));
    sources.emplace_back(std::unique_ptr<Util::IO::Input>(std::move(new 
                                                    Util::IO::OCVCapture())));

    /* Define the protocol parameter */
    protocol.denominate("protocol")
            .describe("Protocol for the video-source capture")
            .characterise(Customisation::Trait::CONFIGURABLE);
    protocol.trigger([this](const std::string &p) 
                     { return onProtocolUpdate(p);});
    expose(protocol);

    /* Define the user parameter */
    user.denominate("user")
        .describe("Optional user name for the source")
        .characterise(Customisation::Trait::CONFIGURABLE);
    user.trigger([this](const std::string &u) 
                 { return onUserUpdate(u);});
    expose(user);

    /* Define the password parameter */
    password.denominate("password")
            .describe("Optional password for the user")
            .characterise(Customisation::Trait::CONFIGURABLE);
    expose(password);

    /* Define the source parameter */
    source.denominate("source")
          .describe("Source for the selected protocol")
          .characterise(Customisation::Trait::CONFIGURABLE);
    source.trigger([this](const std::string &s) 
                   { return onSourceUpdate(s);});
    expose(source);

    /* Define the width parameter */
    width.denominate("width")
         .describe("Width in pixels for the video source")
         .characterise(Customisation::Trait::CONFIGURABLE);
    expose(width);

    /* Define the height parameter */
    height.denominate("height")
          .describe("Height in pixels for the video source")
          .characterise(Customisation::Trait::CONFIGURABLE);
    expose(height);

    /* Define the rotation parameter */
    rotation.denominate("rotation")
            .describe("Rotation in degrees for the video source")
            .characterise(Customisation::Trait::SETTABLE);
    expose(rotation);

    /* Define the mode parameter */
    mode.denominate("mode")
        .describe("Optimal mode (width, height, rotation) for the source")
        .characterise(Customisation::Trait::CONFIGURABLE);
    mode.trigger([this](const std::string &m) 
                 { return onModeUpdate(m);});
    mode = "640x480";
    expose(mode);

    /* Set the protocol whitelist */
    for (auto &s : sources) {
        protocol.allow(s->protocols());
    }
}

Customisation::Error Capture::setup() noexcept {
    Customisation::Error error;
    terminate();
    
    if (next == nullptr) return Customisation::Error::NOT_EXISTING;

    error = static_cast<Customisation::Error>(next->open(protocol, source));
    if (error != Customisation::Error::NONE) return error;
    
    current = next;
            
    if (!static_cast<std::string>(user).empty()) {
        auto err = current->setup(user, password);
        if (err) return static_cast<Customisation::Error>(err);
    }
    
    int w, h, r;

    w = width;
    h = height;
    r = rotation;
    
    auto err = current->setup(w, h, r);
    if (err) return static_cast<Customisation::Error>(err);

    width    = w;
    height   = h;
    rotation = r;

    return error;
}

Error::Type Capture::process(Scene &orig) noexcept {
    cv::Mat image;
    Error::Type error = Error::NOT_EXISTING;

    if (current != nullptr) {
        error = current->read(image);
        if (! error) {
            if (current->projection()) {
                orig.use(std::move(image), *current->projection());
            } else {
                orig.use(std::move(image));
            }
        }
    }

    return error;
}

void Capture::terminate() noexcept {
    if (current != nullptr) {
        current->close();
        current = nullptr;
    }
}

Customisation::Error Capture::onProtocolUpdate(const std::string &p) noexcept {
    for (auto &s : sources) {
        if (s->supports(p)) {
            terminate();
            next = s.get();

            /* Update the source whitelist */
            source.allow();
    
            auto new_src = next->sources();
            if (!new_src.empty()) {
                source.allow(std::set<std::string>(new_src.begin(), 
                                                   new_src.end()));
                /* Default the source to the first one */
                source = *new_src.begin();
            } else {
                static_cast<std::string>(source).clear();
            }

            return Customisation::Error::NONE;
        }
    }

    return Customisation::Error::INVALID_REQUEST;
}

Customisation::Error
    Capture::onSourceUpdate(const std::string &s) noexcept {
    terminate();
    auto error = next->open(protocol, s);

    /* No specific mode by default */ 
    mode.allow();

    /* By default, only allow no rotation and default to this setup */
    rotation.allow();
    rotation.allow(std::set<int>({ 0 }));
    rotation = 0;

    if (!error) {
        auto new_modes = next->modes();
        if (!new_modes.empty()) {
            bool default_mode_set = false;
            for (auto &m : new_modes) { 
                int w, h, r;
                if (string2mode(m, w, h, r)) {
                    if (!default_mode_set) {
                        mode = m;
                        default_mode_set = true;
                    }
                    rotation.allow(r);
                    mode.allow(std::move(m));
                }
            }
        } else {
            mode = "640x480";
        }
        next->close();
    } else {
        mode = "0x0";
    }

    return static_cast<Customisation::Error>(error);
}

Customisation::Error Capture::onUserUpdate(const std::string &u) noexcept {
    if (u.empty()) {
        static_cast<std::string>(password).clear();
    }
    return Customisation::Error::NONE;
}

Customisation::Error Capture::onModeUpdate(const std::string &m) noexcept {
    int w, h, r;
    
    if (!string2mode(m, w, h, r)) {
        ASSERT((false),
               "%s::Capture[%s]::onModeUpdate(): Invalid mode provided %s!",
               value_to_string().c_str(), name().c_str(), m.c_str());
    
        return Customisation::Error::INVALID_VALUE;
    }

    width    = w;
    height   = h;
    rotation = r;

    return Customisation::Error::NONE;
}

}  // namespace Engine
}  // namespace VPP
