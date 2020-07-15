/**
 *
 * @file      vpp/engine/tracker/kalman.hpp
 *
 * @brief     This is the VPP kalman-based predicter and tracker description
 *            file
 *
 * @details   This engine is a kalman predicter aimed at predicting the next
 *            estimated position of zones coupled to a tracker to keep the last
 *            scene status
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
#include "vpp/task/tracker/kalman.hpp"
#include "vpp/task/matcher.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

class Kalman : public VPP::Engine::ForScene {
    public:
        using Matcher = 
                VPP::Task::Matcher::Generic<VPP::Tracker::Kalman::Contexts&,
                                            VPP::Tracker::Kalman::Contexts&,
                                            VPP::Task::Matcher::Estimator::Any>;

        Kalman(Scene &history, std::mutex &synchro,
               std::vector<Zone> *added = nullptr,
               std::vector<Zone> *removed = nullptr) noexcept;
        ~Kalman() noexcept = default;

        Customisation::Error setup() noexcept override;

        Error::Type process(Scene &scene) noexcept override;

        VPP::Tracker::Kalman::Engine           engine;
        VPP::Task::Tracker::Kalman::Prediction prediction;
        VPP::Task::Tracker::Kalman::Correction correction;
        Matcher                               matcher;

    private:
        std::mutex &                          update;
        Scene &                               latest;
        std::vector<Zone> *                   entering;
        std::vector<Zone> *                   leaving;
};

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
