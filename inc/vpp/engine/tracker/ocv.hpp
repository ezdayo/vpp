/**
 *
 * @file      vpp/engine/tracker/ocv.hpp
 *
 * @brief     This is the OCV trackers description file
 *
 * @details   This engine is a generic engine for OCV trackers
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

#include "vpp/error.hpp"
#include "vpp/engine.hpp"
#include "vpp/scene.hpp"
#include "vpp/task/tracker/ocv.hpp"
#include "vpp/task/matcher.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

class OCV : public VPP::Engine::ForScene {
    public:
        using Matcher = 
                VPP::Task::Matcher::Generic<VPP::Tracker::OCV::Contexts&,
                                            VPP::Tracker::OCV::Contexts&,
                                            VPP::Task::Matcher::Estimator::Any>;

        OCV(Scene &history, std::mutex &synchro,
                 std::vector<Zone> *added = nullptr,
                 std::vector<Zone> *removed = nullptr) noexcept;
        ~OCV() noexcept = default;

        Error::Type process(Scene &scene) noexcept override;

        VPP::Tracker::OCV::Engine            engine;
        VPP::Task::Tracker::OCV::Initialiser initialisation;
        VPP::Task::Tracker::OCV::Predicter   estimation;
        Matcher                              matcher;

    private:
        std::mutex &                          update;
        Scene &                               latest;
        std::vector<Zone> *                   entering;
        std::vector<Zone> *                   leaving;
};

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
