/**
 *
 * @file      vpp/engine/capture.hpp
 *
 * @brief     This is the VPP capture engine definition
 *
 * @details   This is always the first engine used in the primary VPP pipeline
 *            for it is the one getting the original stream of images, be it
 *            from an internal camera, a wifi camera or any other network video
 *            stream.
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

#include <set>
#include <string>

#include "customisation/parameter.hpp"
#include "error.hpp"
#include "vpp/scene.hpp"
#include "vpp/engine.hpp"
#include "vpp/util/io/input.hpp"

namespace VPP {
namespace Engine {

class Capture : public Engine::ForScene {
    public:
        Capture() noexcept;
        ~Capture() noexcept = default;

        Customisation::Error setup() noexcept override;
        Error::Type process(Scene &scene) noexcept override;
        void terminate() noexcept override;

        /* Updating the protocol is updating the white list of the other 
         * source */
        PARAMETER(Direct, WhiteListed, Callable, std::string) protocol;

        /* Updating the source is updating the white list for the supported
         * modes */
        PARAMETER(Direct, WhiteListed, Callable, std::string) source;

        /* Clearing the user name is clearing the password field */
        PARAMETER(Direct, None, Callable, std::string) user;
        PARAMETER(Direct, None, Immediate, std::string) password;

        PARAMETER(Direct, None, Immediate, int) width;
        PARAMETER(Direct, None, Immediate, int) height;
        PARAMETER(Direct, WhiteListed, Immediate, int) rotation;

        /* Updating mode is also udpating the width, height and rotation
         * parameters accordingly */
        PARAMETER(Direct, WhiteListed, Callable, std::string) mode;

    private:
        Customisation::Error onProtocolUpdate(const std::string &p) noexcept;
        Customisation::Error onSourceUpdate(const std::string &s) noexcept;
        Customisation::Error onUserUpdate(const std::string &u) noexcept;
        Customisation::Error onModeUpdate(const std::string &m) noexcept;

        std::vector<std::unique_ptr<Util::IO::Input>> sources;
        Util::IO::Input *                             current;
        Util::IO::Input *                             next;
};

}  // namespace Engine
}  // namespace VPP
