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
    : dnj(Util::Task::Mode::Sync),
      similarity(Util::Task::Mode::Sync) {
    similarity.denominate("similarity");
    expose(similarity);

    similarity.filter = ([](const Zone &){ return false; });
    similarity.threshold = 1.0f;

    dnj.denominate("dnj");
    expose(dnj);

    dnj.filter = ([](const Zone &){ return false; });
    dnj.ratio = 0.33f;
    dnj.cross = true;
}

Error::Type Clustering::process(Scene &scene) noexcept {
    auto e = similarity.start(scene);
    if (e != Error::NONE) return e;
    e = similarity.wait();
    if (e != Error::NONE) return e;
    e = dnj.start(scene);
    if (e != Error::NONE) return e;
    e = dnj.wait();

    return e;
}

}  // namespace Engine
}  // namespace VPP
