/**
 *
 * @file      vpp/core/engine.tpl.hpp
 *
 * @brief     This is the VPP engine implementation
 *
 * @details   This is the base class of all pipeline engines.
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

#include "vpp/log.hpp"
#include "vpp/core/engine.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> Engine<Z...>::Engine() noexcept
    : Customisation::Entity("Engine") {}

template <typename ...Z> 
    Error::Type Engine<Z...>::prepare(Scene*& /*s*/, Z*&... /*z*/) noexcept {
    return Error::OK;
}

template <typename ...Z> 
    Error::Type Engine<Z...>::process(Scene &/*s*/, Z&... /*z*/) noexcept {
    return Error::OK;
}

}  // namespace Core
}  // namespace VPP
