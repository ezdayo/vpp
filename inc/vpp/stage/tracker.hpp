/**
 *
 * @file      vpp/stage/tracker.hpp
 *
 * @brief     These is the VPP tracker stage definitions
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

#include "vpp/engine/tracker/history.hpp"
#include "vpp/engine/tracker/kalman.hpp"
#include "vpp/engine/tracker/none.hpp"
#include "vpp/stage.hpp"
#include "vpp/util/observability.hpp"

#include <mutex>

namespace VPP {
namespace Stage {

class Tracker : public Stage::ForScene {
    public:
        Tracker() noexcept;
        ~Tracker() noexcept = default;

        VPP::Engine::Tracker::Kalman  kalman;
        VPP::Engine::Tracker::History history;
        VPP::Engine::Tracker::None    none;

        void snapshot(Scene &s) noexcept;
        void snapshot(std::vector<Zone> &entering,
                      std::vector<Zone> &leaving) noexcept;
        void snapshot(Scene &s, std::vector<Zone> &entering, 
                      std::vector<Zone> &leaving) noexcept;

        virtual Error::Type process(Scene &s) noexcept override;

        Util::Notifier<Scene, std::vector<Zone>, std::vector<Zone>> event;

    protected:
        std::mutex        synchro;
        Scene             latest;
        std::vector<Zone> added;
        std::vector<Zone> removed;
};

}  // namespace Stage
}  // namespace VPP
