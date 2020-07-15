/**
 *
 * @file      vpp/task.hpp
 *
 * @brief     This is the VPP tasks description file
 *
 * @details   This is the definition of the various kinds of tasks that can be
 *            used in the VPP. It is nothing but specialisations of the utility
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

#include <type_traits>
#include <utility>

#include "vpp/log.hpp"
#include "vpp/scene.hpp"
#include "vpp/util/task.hpp"
#include "vpp/types.hpp"

namespace VPP {
namespace Task {

/** Single task */
template <typename T, typename ...E> 
    class Single : public Parametrisable, public Util::Task::Single<T, E...> {
    public:
        using Parent = Util::Task::Single<T, E...>;
        using typename Parent::Mode;
        
        inline explicit Single(const int mode) noexcept 
            : Customisation::Entity("Task"), Parent(mode) {
            ASSERT((mode == Mode::Async) || (mode == Mode::Sync) || 
                   (mode == Mode::Lazy),
                   "%s[%s]::Task(): Invalid mode provided %d!",
                   value_to_string().c_str(), name().c_str(), mode);
        }

        /** Do the actual processing */
        inline Error::Type process(E&... /*e*/) noexcept {
            LOGE("%s[%s]::process(): Process shall be redefined in child "
                 "classes!", value_to_string().c_str(), name().c_str());
            return Error::NOT_EXISTING;
        }
};

}  // namespace Task

namespace Tasks {

/** Task list: runs a list of tasks */
template <typename T, typename L, typename ...E> 
    class List : public Parametrisable, public Util::Tasks::List<T, L, E...>
    {
    public:
        using Parent = Util::Tasks::List<T, L, E...>;
        using typename Parent::Mode;
        using Parent::start;
        using Parent::process;
        using Parent::wait;

        inline explicit List(const int mode) noexcept
            : Customisation::Entity("Tasks"), Parent(mode) {};
        inline ~List() noexcept = default;

        inline Error::Type process(Util::containee_object_t<L> &/*o*/, 
                                   E&... /*e*/) noexcept {
            LOGE("%s[%s]::process(): Process shall be redefined in child "
                 "classes!", value_to_string().c_str(), name().c_str());
            return Error::NOT_EXISTING;
        }
};

/** Task lists: runs two lists X and Y of tasks, running each task along the
 * Y axis, then the X axis (X is row, Y is columns) */
template <typename T, typename X, typename Y, typename ...E> 
    class Lists : public Parametrisable, 
                  public Util::Tasks::Lists<T, X, Y, E...> {
    public:
        using Parent = Util::Tasks::Lists<T, X, Y, E...>;
        using typename Parent::Mode;
        using Parent::start;
        using Parent::process;
        using Parent::wait;

        inline explicit Lists(const int mode) noexcept
            : Customisation::Entity("Tasks"), Parent(mode) {};
        inline ~Lists() noexcept = default;

        inline Error::Type process(Util::containee_object_t<X> &/*xo*/, 
                                   Util::containee_object_t<Y> &/*yo*/, 
                                   E&... /*e*/) noexcept {
            LOGE("%s[%s]::process(): Process shall be redefined in child "
                 "classes!", value_to_string().c_str(), name().c_str());
            return Error::NOT_EXISTING;
        }
};

/* Describing parallel tasks operating on bidimentional tiles in a scene */
template <typename T> 
class Tiled : public Parametrisable, 
              public Util::Tasks::Core<T, Scene &, cv::Rect> {
    public:
        using Parent = Util::Tasks::Core<T, Scene &, cv::Rect>;
        using typename Parent::Mode;
        using Parent::next;
        using Parent::wait;

        inline explicit Tiled(const int mode) noexcept 
            : Customisation::Entity("Tasks"), Parent(mode), frame(), it(),
              tiles_total(0) {
            tile.denominate("tile")
                .describe("The tile geometry to use for the processing");
            expose(tile);

            /* Use default macro-block size for starting */
            tile.width  = 16;
            tile.height = 16;

            stride.denominate("stride")
                  .describe("The stride to use for the processing");
            expose(stride);
    
            /* Use default macro-block stride for starting */
            stride.x = 16;
            stride.y = 16;
        }
 
        inline ~Tiled() noexcept = default;
        
        inline Error::Type start(Scene &s, cv::Rect f) noexcept {
            frame        = std::move(f);
            it           = cv::Rect(frame.tl(), static_cast<cv::Size>(tile));
            tiles_total  = 0;
            cv::Rect roi(it);
            return Parent::start(s, roi);
        }

        Tile   tile;
        Stride stride;

    protected:
        /** Iterator to do things in parallel */
        inline bool next(Scene& /*s*/, cv::Rect &roi) noexcept {
            if (frame.contains(it.tl())) {
                roi = it;
                ++ tiles_total;
    
                /* Compute the next location */
                it.x += stride.x;
                if (!frame.contains(it.tl())) {
                    it.x = frame.x;
                    it.y += stride.y;
                }
                return true;
            }

            return false;
        }
        
        /** Do the actual tiled scene processing */
        inline Error::Type process(Scene& /*s*/, cv::Rect &/*roi*/) noexcept {
            LOGE("%s[%s]::process(): Process shall be redefined in child "
                 "classes!", value_to_string().c_str(), name().c_str());
            return Error::NOT_EXISTING;
        }

        cv::Rect frame;
        cv::Rect it;
        int      tiles_total;
};

}  // namespace Task
}  // namespace VPP
