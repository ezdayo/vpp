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

class Prediction : public Tasks::ForZones {
    public:
        explicit Prediction(const int mode, VPP::Kernel::Kalman &f) noexcept;
        virtual ~Prediction() noexcept = default;
 
        Error::Type start(Scene &s, Zones &zs, float dt) noexcept;
        Error::Type wait() noexcept;

     protected:
        virtual Error::Type process(Scene &s, Zone &z)
            noexcept override final;

    private:
        VPP::Kernel::Kalman &kalman;
        float                dt;
};

class Correction : public Tasks::ForZones {
    public:
        explicit Correction(const int mode, VPP::Kernel::Kalman &f) noexcept;
        virtual ~Correction() noexcept = default;
 
     protected:
        virtual Error::Type process(Scene &s, Zone &z)
            noexcept override final;

    private:
        VPP::Kernel::Kalman &kalman;
};


}  // namespace Kalman
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
