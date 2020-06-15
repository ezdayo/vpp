/**
 *
 * @file      vpp/pipeline.cpp
 *
 * @brief     This is the VPP pipelines implementation
 *
 * @details   This is the base classes of all VPP pipelines.
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

#include "core/pipeline.tpl.hpp"
#include "vpp/pipeline.hpp"

namespace VPP {
namespace Core {

/* Create template implementations */

template<> void Pipeline<>::launch() noexcept {
    Scene scene;
    auto s = &scene;
    return work(s);
}

template<> void Pipeline<>::prepare(Scene *&s) noexcept {
    *s = std::move(Scene());
}

template class Pipeline<>;

template<> void Pipeline<Zone>::launch() noexcept {
    Scene scene;
    Zone  zone;
    auto s = &scene;
    auto z = &zone;
    return work(s, z);
}

template<> void Pipeline<Zone>::prepare(Scene *& /*s*/, Zone *&z) noexcept {
    *z = std::move(Zone());
}

template class Pipeline<Zone>;

template<> 
void Pipeline<Zones>::launch() noexcept {
    Scene scene;
    Zones zones;
    auto s = &scene;
    auto z = &zones;

    return work(s, z);
}

template<> void Pipeline<Zones>::prepare(Scene *& /*s*/, Zones *&z) noexcept {
    *z = std::move(Zones());
}

template class Pipeline<Zones>;

}  // namespace Core
}  // namespace VPP
