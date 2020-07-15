/**
 *
 * @file      vpp/tracker/ocv.hpp
 *
 * @brief     This is the OpenCV trackers implementation file
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

#include <opencv2/core/mat.hpp>

#include "vpp/tracker/ocv.hpp"

namespace VPP {
namespace Tracker {
namespace OCV {

Context::Context(Zone &zone, Zone::Copier &copier,
                 unsigned int sz, Factory *factory) noexcept
    : VPP::Tracker::Context(zone, copier, sz), tracker((*factory)()) {}

void Context::initialise(VPP::View &view) noexcept {
    if (!tracker->init(view.bgr().input(), zone())) {
        invalidate();
    }
}

void Context::predict(VPP::View &view) noexcept {
    if (valid()) {
        cv::Rect2d estimated;
        if ( (!(tracker->update(view.bgr().input(), estimated))) ||
             ((static_cast<cv::Rect >(estimated) & view.frame()).area() == 0) ) {
            invalidate();
            return;
        }
        Zone &z = stack(zone(-1));
        static_cast<cv::Rect &>(z) = estimated;

        z.deproject(view);
    }
}

#define CREATE_FACTORY_FOR(x) \
static cv::Ptr<cv::Tracker> x##Factory() noexcept { \
    return cv::Tracker##x::create(); \
}

CREATE_FACTORY_FOR(Boosting)
CREATE_FACTORY_FOR(CSRT)
CREATE_FACTORY_FOR(GOTURN)
CREATE_FACTORY_FOR(KCF)
CREATE_FACTORY_FOR(MedianFlow)
CREATE_FACTORY_FOR(MIL)
CREATE_FACTORY_FOR(MOSSE)
CREATE_FACTORY_FOR(TLD)

Engine::Engine(const Zone::Copier &c, unsigned int sz) noexcept 
    : VPP::Tracker::Engine<Engine, Context>(c, sz), 
      factories( { { "Boosting",   BoostingFactory },
                   { "CSRT",       CSRTFactory },
                   { "GOTRUN",     GOTURNFactory },
                   { "KCF",        KCFFactory },
                   { "MedianFlow", MedianFlowFactory },
                   { "MIL",        MILFactory },
                   { "MOSSE",      MOSSEFactory },
                   { "TLD",        TLDFactory } }),
      factory(nullptr) {

    std::set<std::string> models;
    for (auto kv : factories) {
        models.emplace(kv.first); 
    }

    tracker.denominate("tracker")
           .describe("The tracker model to use")
           .characterise(Customisation::Trait::SETTABLE);
    tracker.trigger([this](const std::string &t) { 
                           return onTrackerUpdate(t); });
    tracker.allow(std::move(models));
    Customisation::Entity::expose(tracker);
    tracker = "MIL";
}
        
Customisation::Error Engine::setup() noexcept {
    return clear(); 
}
    
Customisation::Error Engine::clear() noexcept {
    storage.clear();
    return Customisation::Error::NONE;
}

void Engine::prepare(Zones &zs) noexcept {
    Parent::prepare(zs, factory);
}

Customisation::Error Engine::onTrackerUpdate(const std::string &t) noexcept {
    ASSERT(factories.find(t) != factories.end(),
           "Tracker::OCV::Engine::onTrackerUpdate(): Threre is no %s model!",
           t.c_str());
    
    auto it = factories.find(t);
    if (it == factories.end()) {
        return Customisation::Error::INVALID_VALUE;
    }

    for (auto &c : storage) {
        c.invalidate();
    }

    factory = &it->second;
    return Customisation::Error::NONE;
}

}  // namespace OCV
}  // namespace Tracker
}  // namespace VPP
