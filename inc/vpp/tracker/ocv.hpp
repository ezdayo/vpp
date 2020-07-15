/**
 *
 * @file      vpp/tracker/ocv.hpp
 *
 * @brief     This is the OpenCV trackers description file
 *
 * @details   These trackers are using the trackers found in the OpenCV contrib
 *            i.e. MIL, Boosting, MedianFlow, TLD, KCF, GOTURN, MOSSE, and CSRT
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

#include <opencv2/tracking/tracker.hpp>
#include <unordered_map>

#include "customisation.hpp"
#include "vpp/tracker.hpp"
#include "vpp/zone.hpp"

namespace VPP {
namespace Tracker {
namespace OCV {
        
using Factory = std::function<cv::Ptr<cv::Tracker>() noexcept>;

class Context : public VPP::Tracker::Context {
    public:
        explicit Context(Zone &zone, Zone::Copier &copier,
                         unsigned int sz, Factory *factory) noexcept;
        ~Context() noexcept = default;

        /* Initialising the tracker */ 
        void initialise(VPP::View &view) noexcept;

        /* Stack the predicted zone atop */
        void predict(VPP::View &view) noexcept;

    protected:
        cv::Ptr<cv::Tracker> tracker;
};

using Contexts = std::vector<std::reference_wrapper<Context>>;

class Engine : public VPP::Tracker::Engine<Engine, Context> {
    public:
        using Parent = VPP::Tracker::Engine<Engine, Context>;

        explicit Engine(const Zone::Copier &c, unsigned int sz = 2) noexcept;
        ~Engine() noexcept = default;

        Customisation::Error setup() noexcept override;

        Customisation::Error clear() noexcept override;

        /* The tracker model to use */
        PARAMETER(Direct, WhiteListed, Callable, std::string) tracker;

        void prepare(Zones &zs) noexcept;

    protected:
        Customisation::Error onTrackerUpdate(const std::string &t) noexcept;

        std::unordered_map<std::string, Factory> factories;
        Factory *                                factory;
};

}  // namespace OCV
}  // namespace Tracker
}  // namespace VPP
