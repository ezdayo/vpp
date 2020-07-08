/**
 *
 * @file      vpp/task/kernel/kalman.hpp
 *
 * @brief     These are various tasks to efficiently use Kalman kernels in a
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
#include "vpp/kernel/kalman.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Kernel {
namespace Kalman {

class Prediction
    : public VPP::Tasks::List<Prediction, VPP::Kernel::Kalman::Contexts&,
                              Scene&> {
    public:
        using Parent = 
                VPP::Tasks::List<Prediction, VPP::Kernel::Kalman::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit Prediction(const int mode,
                            VPP::Kernel::Kalman::Engine &e) noexcept;
        virtual ~Prediction() noexcept = default;
 
        Error::Type start(Scene &s, float dt,
                          VPP::Kernel::Kalman::Contexts &ctx) noexcept;
        Error::Type start(Scene &s, float dt) noexcept;
        Error::Type wait() noexcept;

        Error::Type process(VPP::Kernel::Kalman::Context &c, Scene &s) noexcept;

    private:
        VPP::Kernel::Kalman::Contexts contexts;
        VPP::Kernel::Kalman::Engine & kalman;
        float                         dt;
};

class Correction 
    : public VPP::Tasks::List<Correction, VPP::Kernel::Kalman::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<Correction, VPP::Kernel::Kalman::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;
        
        explicit Correction(const int mode,
                            VPP::Kernel::Kalman::Engine &e) noexcept;
        virtual ~Correction() noexcept = default;
 
        Error::Type start(Scene &s,
                          VPP::Kernel::Kalman::Contexts &ctx) noexcept;
        Error::Type start(Scene &s) noexcept;

        Error::Type process(VPP::Kernel::Kalman::Context &c, Scene &s) noexcept;

    private:
        VPP::Kernel::Kalman::Contexts contexts;
        VPP::Kernel::Kalman::Engine & kalman;
};


}  // namespace Kalman
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
