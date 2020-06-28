/**
 *
 * @file      vpp/engine/tracker/none.cpp
 *
 * @brief     This is the no VPP history tracker implementation file
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

#include "vpp/engine/tracker/none.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

None::None(Scene &history) noexcept 
    : VPP::Engine::ForScene(), latest(history) {}
        
None::~None() noexcept = default;

Customisation::Error None::setup() noexcept {
    latest = std::move(Scene());

    return Customisation::Error::NONE;
}
        
Error::Type None::process(Scene &/*scene*/) noexcept {
    return Error::NONE;
}

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
