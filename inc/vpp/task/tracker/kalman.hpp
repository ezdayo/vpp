/**
 *
 * @file      vpp/task/tracker/kalman.hpp
 *
 * @brief     These are various tasks to efficiently use Kalman trackers in a
 *            scene
 *
 * @details   This is a collection of tasks for helping in the efficient
 *            implementation of a Kalman predict and tracking in a scene
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

#include "customisation/parameter.hpp"
#include "vpp/tracker/kalman.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Tracker {
namespace Kalman {

class Prediction
    : public VPP::Tasks::List<Prediction, VPP::Tracker::Kalman::Contexts&,
                              Scene&> {
    public:
        using Parent = 
                VPP::Tasks::List<Prediction, VPP::Tracker::Kalman::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit Prediction(const int mode,
                            VPP::Tracker::Kalman::Engine &e) noexcept;
        virtual ~Prediction() noexcept = default;
 
        Error::Type start(Scene &s, float dt,
                          VPP::Tracker::Kalman::Contexts &ctx) noexcept;
        Error::Type start(Scene &s, float dt) noexcept;
        Error::Type wait() noexcept;

        Error::Type process(VPP::Tracker::Kalman::Context &c, Scene &s) noexcept;

    private:
        VPP::Tracker::Kalman::Contexts contexts;
        VPP::Tracker::Kalman::Engine & kalman;
        float                         dt;
};

class Correction 
    : public VPP::Tasks::List<Correction, VPP::Tracker::Kalman::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<Correction, VPP::Tracker::Kalman::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;
        
        explicit Correction(const int mode,
                            VPP::Tracker::Kalman::Engine &e) noexcept;
        virtual ~Correction() noexcept = default;
 
        Error::Type start(Scene &s,
                          VPP::Tracker::Kalman::Contexts &ctx) noexcept;
        Error::Type start(Scene &s) noexcept;

        Error::Type process(VPP::Tracker::Kalman::Context &c, Scene &s) noexcept;

    private:
        VPP::Tracker::Kalman::Contexts contexts;
        VPP::Tracker::Kalman::Engine & kalman;
};


}  // namespace Kalman
}  // namespace Tracker
}  // namespace Task
}  // namespace VPP
