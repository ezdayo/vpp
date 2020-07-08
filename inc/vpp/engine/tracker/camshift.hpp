/**
 *
 * @file      vpp/engine/tracker/camshift.hpp
 *
 * @brief     This is the VPP camshift-based predicter and tracker description
 *            file
 *
 * @details   This engine is a camshift predicter aimed at predicting the next
 *            estimated position of zones coupled to a histogram comparisons to
 *            check if the predicted zone is relevant or not
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
#include "vpp/task/kernel/histogram.hpp"
#include "vpp/task/matcher.hpp"

namespace VPP {
namespace Engine {
namespace Tracker {

class CamShift : public VPP::Engine::ForScene {
    public:
        using Matcher = 
                VPP::Task::Matcher::Generic<VPP::Kernel::Histogram::Contexts&,
                                            VPP::Kernel::Histogram::Contexts&,
                                            VPP::Task::Matcher::Estimator::Any>;

        CamShift(Scene &history, std::mutex &synchro,
                 std::vector<Zone> *added = nullptr,
                 std::vector<Zone> *removed = nullptr) noexcept;
        ~CamShift() noexcept = default;

        Error::Type process(Scene &scene) noexcept override;

        VPP::Kernel::Histogram::Engine            engine;
        VPP::Task::Kernel::Histogram::Initialiser initialisation;
        VPP::Task::Kernel::Histogram::CamShift    estimation;
        Matcher                                   matcher;

    private:
        std::mutex &                          update;
        Scene &                               latest;
        std::vector<Zone> *                   entering;
        std::vector<Zone> *                   leaving;
};

}  // namespace Tracker
}  // namespace Engine
}  // namespace VPP
