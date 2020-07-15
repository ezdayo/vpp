/**
 *
 * @file      vpp/task/tracker/ocv.hpp
 *
 * @brief     These are various tasks to efficiently use OCV trackers in a scene
 *
 * @details   This is a collection of tasks for helping in the efficient
 *            implementation of OCV-based trackers in a scene
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
#include "vpp/tracker/ocv.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Tracker {
namespace OCV {

class Initialiser
    : public VPP::Tasks::List<Initialiser, VPP::Tracker::OCV::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<Initialiser, 
                                 VPP::Tracker::OCV::Contexts&, Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit Initialiser(const int mode,
                             VPP::Tracker::OCV::Engine &e) noexcept;
        ~Initialiser() noexcept = default;

        Error::Type start(Scene &s, Zones &zs, 
                          VPP::Tracker::OCV::Contexts &ctx) noexcept;
        Error::Type start(Scene &s, Zones &zs) noexcept;

        Error::Type process(VPP::Tracker::OCV::Context &c,
                            Scene &s) noexcept;

    private:
        VPP::Tracker::OCV::Contexts contexts;
        VPP::Tracker::OCV::Engine & ocv;
};

class Predicter 
    : public VPP::Tasks::List<Predicter, VPP::Tracker::OCV::Contexts&,
                              Scene&> {
    public:
        using Parent =
                VPP::Tasks::List<Predicter, VPP::Tracker::OCV::Contexts&,
                                 Scene&>;
        using typename Parent::Mode;
        using Parent::process;
        using Parent::next;

        explicit Predicter(const int mode,
                           VPP::Tracker::OCV::Engine &e) noexcept;
        ~Predicter() noexcept = default;

        Error::Type start(Scene &s,
                          VPP::Tracker::OCV::Contexts &ctx) noexcept;
        Error::Type start(Scene &s) noexcept;

        Error::Type process(VPP::Tracker::OCV::Context &c,
                            Scene &scene) noexcept;

    private:
        VPP::Tracker::OCV::Contexts contexts;
        VPP::Tracker::OCV::Engine & ocv;
};

}  // namespace OCV
}  // namespace Tracker
}  // namespace Task
}  // namespace VPP
