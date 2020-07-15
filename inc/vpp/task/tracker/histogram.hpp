/**
 *
 * @file      vpp/task/tracker/histogram.hpp
 *
 * @brief     These are various tasks to efficiently use Histograms trackers in 
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
#include "vpp/tracker/histogram.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Tracker {
namespace Histogram {

class Initialiser
    : public VPP::Tasks::List<Initialiser, VPP::Tracker::Histogram::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<Initialiser, 
                                 VPP::Tracker::Histogram::Contexts&, Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit Initialiser(const int mode,
                             VPP::Tracker::Histogram::Engine &e) noexcept;
        ~Initialiser() noexcept = default;

        Error::Type start(Scene &s, Zones &zs, 
                          VPP::Tracker::Histogram::Contexts &ctx) noexcept;
        Error::Type start(Scene &s, Zones &zs) noexcept;

        Error::Type process(VPP::Tracker::Histogram::Context &c,
                            Scene &s) noexcept;

    private:
        VPP::Tracker::Histogram::Contexts contexts;
        VPP::Tracker::Histogram::Engine & histogram;
};

class CamShift 
    : public VPP::Tasks::List<CamShift, VPP::Tracker::Histogram::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<CamShift, VPP::Tracker::Histogram::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit CamShift(const int mode,
                          VPP::Tracker::Histogram::Engine &e) noexcept;
        ~CamShift() noexcept = default;

        Error::Type start(Scene &s,
                          VPP::Tracker::Histogram::Contexts &ctx) noexcept;
        Error::Type start(Scene &s) noexcept;

        Error::Type process(VPP::Tracker::Histogram::Context &c,
                            Scene &scene) noexcept;

        /* Camshift epsilon value to stop searching */
        PARAMETER(Direct, Saturating, Callable, float)  epsilon;
        
        /* Camshift maximum number of iterations */
        PARAMETER(Direct, Saturating, Callable, int)    iterations;

        /* Camshift threshold for accepting an histogram match */
        PARAMETER(Direct, Saturating, Immediate, float) threshold;

    private:
        Customisation::Error onEpsilonUpdate(const float &e) noexcept;
        Customisation::Error onIterationsUpdate(const int &i) noexcept;

        VPP::Tracker::Histogram::Contexts contexts;
        VPP::Tracker::Histogram::Engine & histogram;
        cv::TermCriteria                 term;
};

}  // namespace Histogram
}  // namespace Tracker
}  // namespace Task
}  // namespace VPP
