/**
 *
 * @file      vpp/engine/bridge.hpp
 *
 * @brief     This is the VPP bridge engine definition
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

#pragma once

#include <mutex>

#include "error.hpp"
#include "vpp/scene.hpp"
#include "vpp/engine.hpp"

namespace VPP {
namespace Engine {

template <typename ...Z> class Bridge : public Core::Engine<Z...> {
    public:
        Bridge() noexcept;
        ~Bridge() noexcept = default;

        /* Forwarding a scene or a scene with a list of zone references to this
         * bridge. If the scene shall not be copied, then use std::move when
         * calling this method */
        void forward(Scene scn) noexcept;
        void forward(Zones zs) noexcept;
        void forward(Zone &z) noexcept;

        Scene &scene() noexcept;
        bool empty() noexcept;

        Customisation::Error setup() noexcept override;
        /* Prepare returns not existing if there is nothing more to process */
        Error::Type prepare(Scene*& s, Z*&... z) noexcept override;
        void terminate() noexcept override;

    private:
        std::mutex  access;
        int         rd, wr;
        Scene       scenes[2];
        Zones       zones[2]; 
};

template <> class Bridge<> : public Core::Engine<> {
    public:
        Bridge() noexcept;
        ~Bridge() noexcept = default;

        /* Forwarding a scene or a scene with a list of zone references to this
         * bridge. If the scene shall not be copied, then use std::move when
         * calling this method */
        void forward(Scene scn) noexcept;
        bool empty() noexcept;

        Customisation::Error setup() noexcept override;
        /* Prepare returns not existing if there is nothing more to process */
        Error::Type prepare(Scene*& s) noexcept override;
        void terminate() noexcept override;

    private:
        std::mutex  access;
        int         rd, wr;
        Scene       scenes[2];
};

/* Describing a bridge for handling a full scene */
using BridgeForScene = Bridge<>;

/* Describing a bridge for handling squentially single zones in a scene */
using BridgeForZone = Bridge<Zone>;

/* Describing a bridge for handling multiple zones in a scene */
using BridgeForZones = Bridge<Zones>;

}  // namespace Engine
}  // namespace VPP
