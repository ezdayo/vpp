/**
 *
 * @file      vpp/task/kernel/histogram.hpp
 *
 * @brief     These are various tasks to efficiently use Histograms kernels in 
 *            a scene
 *
 * @details   This is a collection of tasks for helping in the efficient
 *            implementation of a Histogram-based algorithms in a scene
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
#include "vpp/kernel/histogram.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Kernel {
namespace Histogram {

class Initialiser
    : public VPP::Tasks::List<Initialiser, VPP::Kernel::Histogram::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<Initialiser, VPP::Kernel::Histogram::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit Initialiser(const int mode,
                             VPP::Kernel::Histogram::Engine &e) noexcept;
        ~Initialiser() noexcept = default;

        Error::Type start(Scene &s, Zones &zs, 
                          VPP::Kernel::Histogram::Contexts &ctx) noexcept;
        Error::Type start(Scene &s, Zones &zs) noexcept;

        Error::Type process(VPP::Kernel::Histogram::Context &c,
                            Scene &s) noexcept;

    private:
        VPP::Kernel::Histogram::Contexts contexts;
        VPP::Kernel::Histogram::Engine & histogram;
};

class CamShift 
    : public VPP::Tasks::List<CamShift, VPP::Kernel::Histogram::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<CamShift, VPP::Kernel::Histogram::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit CamShift(const int mode,
                          VPP::Kernel::Histogram::Engine &e) noexcept;
        ~CamShift() noexcept = default;

        Error::Type start(Scene &s,
                          VPP::Kernel::Histogram::Contexts &ctx) noexcept;
        Error::Type start(Scene &s) noexcept;

        Error::Type process(VPP::Kernel::Histogram::Context &c,
                            Scene &scene) noexcept;

    private:
        VPP::Kernel::Histogram::Contexts contexts;
        VPP::Kernel::Histogram::Engine & histogram;
        cv::TermCriteria                 term;
};

}  // namespace Histogram
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
