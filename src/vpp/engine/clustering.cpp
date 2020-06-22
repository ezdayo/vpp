/**
 *
 * @file      vpp/engine/clustering.cpp
 *
 * @brief     This is a generic clustering engine
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

#include "vpp/log.hpp"
#include "vpp/engine/clustering.hpp"

namespace VPP {
namespace Engine {

Clustering::Clustering() noexcept
    : dnj(Util::Task::Mode::Sync)
#ifdef VPP_HAS_SIMILARITY_CLUSTERING_SUPPORT
      , similarity(Util::Task::Mode::Sync)
#endif
      {
#ifdef VPP_HAS_SIMILARITY_CLUSTERING_SUPPORT
    similarity.denominate("similarity");
    expose(similarity);

    similarity.filter = ([](const Zone &){ return false; });
    similarity.threshold = 1.0f;
#endif

    dnj.denominate("dnj");
    expose(dnj);

    dnj.filter = ([](const Zone &){ return false; });
    dnj.ratio = 0.33f;
    dnj.cross = true;
}

Error::Type Clustering::process(Scene &scene) noexcept {
#ifdef VPP_HAS_SIMILARITY_CLUSTERING_SUPPORT
    auto err_sim = similarity.start(scene);
    if (err_sim != Error::NONE) return err_sim;
    err_sim = similarity.wait();
    if (err_sim != Error::NONE) return err_sim;
#endif

    auto err_dnj = dnj.start(scene);
    if (err_dnj != Error::NONE) return err_dnj;
    err_dnj = dnj.wait();

    return err_dnj;
}

}  // namespace Engine
}  // namespace VPP
