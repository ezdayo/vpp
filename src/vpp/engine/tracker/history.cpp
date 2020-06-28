/**
 *
 * @file      vpp/engine/tracker/history.cpp
 *
 * @brief     This is the VPP history tracker implementation file
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

#include "vpp/engine/tracker/history.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

History::History(Scene &history) noexcept 
    : VPP::Engine::ForScene(), latest(history) {}
        
History::~History() noexcept = default;

Customisation::Error History::setup() noexcept {
    latest = std::move(Scene());

    return Customisation::Error::NONE;
}
        
Error::Type History::process(Scene &scene) noexcept {
    latest = std::move(scene.remember());
    return Error::NONE;
}

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
