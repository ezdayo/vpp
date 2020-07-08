/**
 *
 * @file      vpp/stage/tracker.cpp
 *
 * @brief     These is the VPP tracker stage implementation
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

#include "vpp/stage/tracker.hpp"

namespace VPP {
namespace Stage {

        
Tracker::Tracker() noexcept 
    : ForScene(true), kalman(latest, synchro, &added, &removed),
      history(latest, synchro), none(latest), event(), synchro(), latest(), 
      added(), removed() {
    use("none",    none);
    use("history", history);
    use("kalman",  kalman);
}

void Tracker::snapshot(Scene &s) noexcept {
    std::lock_guard<std::mutex> lock(synchro);
    s = std::move(latest.remember());
}

void Tracker::snapshot(std::vector<Zone> &entering,
                      std::vector<Zone> &leaving) noexcept {
    std::lock_guard<std::mutex> lock(synchro);
    entering = added;
    leaving  = removed;
}

void Tracker::snapshot(Scene &s, std::vector<Zone> &entering, 
                      std::vector<Zone> &leaving) noexcept {
    std::lock_guard<std::mutex> lock(synchro);
    s = std::move(latest.remember());
    entering = added;
    leaving  = removed;
}

Error::Type Tracker::process(Scene &s) noexcept {
    auto error = ForScene::process(s);
        
    std::lock_guard<std::mutex> lock(synchro);
    event.signal(latest, added, removed, error);
    
    return error;
}

}  // namespace Stage
}  // namespace VPP
