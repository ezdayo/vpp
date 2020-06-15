/**
 *
 * @file      vpp/engine/bridge.cpp
 *
 * @brief     This is the VPP bridge engine implementation
 *
 * @details   This is an engine aimed at adapting a pipeline to another form of
 *            pipeline, such as binding a Pipeline<Scene> to a Pipeline<Scene,
 *            Zones>, or to a Pipeline<Scene, Zone>, or even to make a tee
 *            connection for making two different kinds of processing in
 *            parallel.
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

#include "vpp/log.hpp"
#include "vpp/engine/bridge.hpp"

namespace VPP {
namespace Engine {

template <typename ...Z> Bridge<Z...>::Bridge() noexcept : 
    Core::Engine<Z...>(), access(), rd(0), wr(0) {}

template <typename ...Z> void Bridge<Z...>::forward(Scene scn) noexcept {
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
    std::lock_guard<std::mutex> lock(access);

    if (rd == wr) {
        wr = (wr+1)%2;
    }

    scenes[wr] = std::move(scn); 
}

template <typename ...Z> void Bridge<Z...>::forward(Zones zs) noexcept {
        
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
    std::lock_guard<std::mutex> lock(access);

    zones[wr] = std::move(zs);
}

template <typename ...Z> void Bridge<Z...>::forward(Zone &z) noexcept {
        
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
    std::lock_guard<std::mutex> lock(access);

    zones[wr].emplace_back(z);
}

template <typename ...Z> Scene& Bridge<Z...>::scene() noexcept {
    std::lock_guard<std::mutex> lock(access);
    return scenes[wr];
}

template <typename ...Z> bool Bridge<Z...>::empty() noexcept {
    std::lock_guard<std::mutex> lock(access);
    return ( (rd == wr) && (zones[rd].empty()) );
}

template <typename ...Z> Customisation::Error Bridge<Z...>::setup() noexcept {
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
    std::lock_guard<std::mutex> lock(access);
    rd = 0;
    wr = 0;
    scenes[0] = Scene();
    scenes[1] = Scene();
    zones[0].clear();
    zones[1].clear();

    return Customisation::Error::NONE;
}

template <typename ...Z> void Bridge<Z...>::terminate() noexcept {
    setup();
}

template<>
Error::Type Bridge<Zone>::prepare(Scene*& scn, Zone*& z) noexcept {
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
     std::lock_guard<std::mutex> lock(access);

    if (rd != wr) {
        scenes[rd] = Scene();
        zones[rd].clear();
        rd = wr;
    }

    if  (zones[rd].empty()) {
        return Error::NOT_READY;
    }

    scn = &scenes[rd];
    z   = &zones[rd].front().get();
    zones[rd].erase(zones[rd].begin());
        
    return Error::NONE;
}

template<>
Error::Type Bridge<Zones>::prepare(Scene*& scn, Zones*& zs) noexcept {
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
     std::lock_guard<std::mutex> lock(access);

    if (rd != wr) {
        scenes[rd] = Scene();
        zones[rd].clear();
        rd = wr;
    }

     if  (zones[rd].empty()) {
        return Error::NOT_READY;
    }

    scn = &scenes[rd];
    /* After this the zones[rd] is empty! */
    *zs  = std::move(zones[rd]);
 
    return Error::NONE;
}

/* Fully specialised bridge for full scenes */
Bridge<>::Bridge() noexcept : Core::Engine<>(), access(), rd(0), wr(0) {}

void Bridge<>::forward(Scene scn) noexcept {
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
    std::lock_guard<std::mutex> lock(access);

    if (rd == wr) {
        wr = (wr+1)%2;
    }

    scenes[wr] = std::move(scn); 
}

bool Bridge<>::empty() noexcept {
    std::lock_guard<std::mutex> lock(access);
    return (rd == wr);
}

Customisation::Error Bridge<>::setup() noexcept {
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
    std::lock_guard<std::mutex> lock(access);
    rd = 0;
    wr = 0;
    scenes[0] = Scene();
    scenes[1] = Scene();

    return Customisation::Error::NONE;
}

void Bridge<>::terminate() noexcept {
    setup();
}

Error::Type Bridge<>::prepare(Scene*& scn) noexcept {
    /* Inside a lock_guard scoped block, as we need to access bridge storage
     * variables */
     std::lock_guard<std::mutex> lock(access);

    if (rd == wr) {
        return Error::NOT_READY;
    }
        
    scenes[rd] = Scene();    
    rd  = wr;
    scn = &scenes[rd];
        
    return Error::NONE;
}

/* Create template implementations */
template class Bridge<Zone>;
template class Bridge<Zones>;

}  // namespace Engine
}  // namespace VPP

