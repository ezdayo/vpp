/**
 *
 * @file      vpp/core/stage.hpp
 *
 * @brief     This is the VPP pipeline core stage description file
 *
 * @details   This file describes the general structure of a stage, i.e. the
 *            entity responsible of a single atomic step of the VPP. 
 *            Stages are piped together to make a pipeline(!), and each stage
 *            embeds at least one engine to perform its duty. Stages can be
 *            bypassed on request.
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

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "customisation/entity.hpp"
#include "customisation/parameter.hpp"
#include "vpp/core/engine.hpp"
#include "vpp/error.hpp"
#include "vpp/scene.hpp"
#include "vpp/util/observability.hpp"

namespace VPP {
namespace Core {

template <typename ...Z> class Stage : public Parametrisable {
    public:
        using Engine = VPP::Core::Engine<Z...>;

        /* Update allows runtime update of the engine */
        explicit Stage(bool update) noexcept;

        /** Stages cannot be copied nor moved */
        Stage(const Stage& other) = delete;
        Stage(Stage&& other) = delete;
        Stage& operator=(const Stage& other) = delete;
        Stage& operator=(Stage&& other) = delete;
        virtual ~Stage() noexcept = default;

        /* Stage bypass handling */
        void bypass(bool yes) noexcept;
        PARAMETER(Direct, None, Callable, bool) bypassed;
        
        /* Stage enable handling */
        void disable(bool yes) noexcept;
        PARAMETER(Direct, None, Callable, bool) disabled;
         
        /* Define and use the engines */
        Customisation::Error use(const std::string id, Engine &eng) noexcept;
        Customisation::Error use(const std::string &id) noexcept;
        PARAMETER(None, WhiteListed, Callable, std::string) engine;

        /* Processing a full scene */
        Error::Type prepare(Scene*& s, Z*&...z) noexcept;
        Error::Type process(Scene &s, Z&...z) noexcept;

        std::function<bool (const Scene &, const Z&...) noexcept> filter;
        Util::Notifier<Scene, Z...> broadcast;

    private:
        bool skipped;
        bool runpdatable;
        Customisation::Error onBypassedUpdate(const bool &yes) noexcept;
        Customisation::Error onDisabledUpdate(const bool &yes) noexcept;
        
        std::unordered_map<std::string, std::reference_wrapper<Engine>> engines;
        Engine *pEngine;
        Customisation::Error onEngineUpdate(const std::string &id) noexcept;

        std::mutex suspend;
};
 
}  // namespace Core
}  // namespace VPP
