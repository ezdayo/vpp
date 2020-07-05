/**
 *
 * @file      vpp/task/clustering.hpp
 *
 * @brief     These are all the various VPP clustering tasks
 *
 * @details   This is a collection of tasks for clustering zones along different
 *            algorithms
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

#include <mutex>

#include "customisation/parameter.hpp"
#include "vpp/config.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Clustering {

class DilateAndJoin : public VPP::Task::Single<DilateAndJoin, Scene&> {
    public:
        using Parent = VPP::Task::Single<DilateAndJoin, Scene&>;
        using typename Parent::Mode;
        using Parent::start;
        using Parent::wait;

        explicit DilateAndJoin(const int mode) noexcept;
        virtual ~DilateAndJoin() noexcept = default;

        /* Zone filter to be applied prior to clustering */
        Scene::ZoneFilter filter;

        /* Dilatation ratio : dilate when > 0, contract when < 0 */
        PARAMETER(Direct, Bounded, Immediate, float) ratio;

        /* Applying cross dilatation. i.e. dilate witdh by a ratio of the height
         * and the height by a ratio of the width, when cross is true. Otherwise
         * a standard dilatation is applied */
        PARAMETER(Direct, None, Immediate, bool) cross;

        Error::Type process(Scene &scn) noexcept;
};

#ifdef VPP_HAS_SIMILARITY_CLUSTERING_SUPPORT
class Similarity : public VPP::Task::Single<Similarity, Scene&> {
    public:
        using Parent = VPP::Task::Single<Similarity, Scene&>;
        using typename Parent::Mode;
        using Parent::start;
        using Parent::wait;

        explicit Similarity(const int mode) noexcept;
        virtual ~Similarity() noexcept = default;

        /* Zone filter to be applied prior to clustering */
        Scene::ZoneFilter filter;

        /* Similarity threshold, the smaller the pickier it becomes */
        PARAMETER(Direct, None, Immediate, double) threshold;

        Error::Type process(Scene &scn) noexcept;
};
#endif

}  // namespace Clustering
}  // namespace Task
}  // namespace VPP
