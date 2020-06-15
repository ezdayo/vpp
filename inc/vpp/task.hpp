/**
 *
 * @file      vpp/task.hpp
 *
 * @brief     This is the VPP tasks description file
 *
 * @details   This is the definition of the various kinds of tasks that can be
 *            used in the VPP. It is nothing but specialisations of the core
 *            task templates.
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

#include "vpp/core/task.hpp"
#include "vpp/scene.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace Task {

/* Describing a task for handling a full scene */
using ForScene = Core::Task<>;

/* Describing a task for handling a single zone in a scene */
using ForZone  = Core::Task<Zone>;

/* Describing a task for handling multiple zones in a scene */
using ForZones = Core::Task<Zones>;

}  // namespace Tasks

namespace Tasks {

/* Describing parallel tasks for each zones of a scene */
class ForScene : public Core::Tasks<Zone> {
    public:

        using Util::Task::Mode;
        using Util::Task::wait;

        explicit ForScene(const int mode) noexcept;
        virtual ~ForScene() noexcept = default;
        
        Error::Type start(Scene &s) noexcept;

    protected:
        virtual Error::Type next(Scene &s, Zone &z)
            noexcept override final;

    private:
        Zones           zones;
        Zones::iterator it;
};

/* Describing parallel tasks for each provided zones of a scene */
class ForZones : public Core::Tasks<Zone, Zones> {
    public:

        using Util::Task::Mode;
        using Util::Task::wait;

        explicit ForZones(const int mode) noexcept;
        virtual ~ForZones() noexcept = default;
        
        Error::Type start(Scene &s, Zones &zs) noexcept;

    protected:
        virtual Error::Type next(Scene &s, Zone &z, Zones &zs) 
            noexcept override final;

    private:
        Zones::iterator it;
};

/* Describing parallel tasks operating on tiles in a scene */
class Tiled : public Core::Tasks<cv::Rect, cv::Rect> {
    public:

        using Util::Task::Mode;
        using Util::Task::wait;

        explicit Tiled(const int mode) noexcept;
        virtual ~Tiled() noexcept = default;
        
        Error::Type start(Scene &s, cv::Rect &frame) noexcept;

        Tile   tile;
        Stride stride;

    protected:
        virtual Error::Type next(Scene &s, cv::Rect &roi, cv::Rect &frame)
            noexcept override final;
        int tiles_total;

    private:
        cv::Rect it;
};

}  // namespace Tasks
}  // namespace VPP
