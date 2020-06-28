/**
 *
 * @file      vpp/engine/tracker/history.hpp
 *
 * @brief     This is the VPP history tracker description file
 *
 * @details   This engine is a simple tracker to keep the last scene status
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

#include "vpp/engine.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

class History : public VPP::Engine::ForScene {
    public:
        explicit History(Scene &history) noexcept;
        ~History() noexcept;

        Customisation::Error setup() noexcept override;
        Error::Type process(Scene &scene) noexcept override;

    private:
        Scene &latest;
};

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
