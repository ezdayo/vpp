/**
 *
 * @file      vpp/task/mser.cpp
 *
 * @brief     This is the maximally stable extremal region (MSER) task
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

#include <opencv2/imgproc.hpp>

#include "vpp/log.hpp"
#include "vpp/task/mser.hpp"

namespace VPP {
namespace Task {

MSER::MSER(const int mode) noexcept 
    : Parent(mode), filter(nullptr), core(static_cast<cv::MSER *>(nullptr)) {
    
    delta.denominate("delta")
         .describe("Indice-delta for comparing size difference")
         .characterise(Customisation::Trait::CONFIGURABLE);
    expose(delta);
    delta = 5;

    min_area.denominate("min_area")
            .describe("Pruning the area which is smaller than this threshold")
            .characterise(Customisation::Trait::CONFIGURABLE);
    expose(min_area);
    min_area = 60;

    max_area.denominate("max_area")
            .describe("Pruning the area which is bigger than this threshold")
            .characterise(Customisation::Trait::CONFIGURABLE);
    expose(max_area);
    max_area = 14400;

    max_variation.denominate("max_variation")
                 .describe("Pruning the area which variation is larger")
                 .characterise(Customisation::Trait::CONFIGURABLE);
    expose(max_variation);
    max_variation = 0.25;

    min_diversity.denominate("min_diversity")
                 .describe("Cut off MSER with diversity lower than this")
                 .characterise(Customisation::Trait::CONFIGURABLE);
    expose(min_diversity);
    min_diversity = 0.2;

    max_evolution.denominate("max_evolution")
                 .describe("The evolution step of colour images")
                 .characterise(Customisation::Trait::CONFIGURABLE);
    expose(max_evolution);
    max_evolution = 200;

    threshold_area.denominate("threshold_area")
                  .describe("The area threshold causing a reinitialisation")
                  .characterise(Customisation::Trait::CONFIGURABLE);
    expose(threshold_area);
    threshold_area = 1.01;

    min_margin.denominate("min_margin")
              .describe("Too small margin threshold gor colour images")
              .characterise(Customisation::Trait::CONFIGURABLE);
    expose(min_margin);
    min_margin = 0.003;

    edge_blur_size.denominate("edge_blur_size")
                  .describe("The apreture size for edge blur")
                  .characterise(Customisation::Trait::CONFIGURABLE);
    expose(edge_blur_size);
    edge_blur_size = 5;
}

Customisation::Error MSER::setup() noexcept {
    core = cv::MSER::create(delta, min_area, max_area, max_variation,
                            min_diversity, max_evolution, threshold_area,
                            min_margin, edge_blur_size);

    return Customisation::Error::NONE;
}

void MSER::terminate() noexcept {
    core = static_cast<cv::MSER *>(nullptr);
}

Error::Type MSER::process(Scene &scene) noexcept {
    std::vector<std::vector<cv::Point> > areas;
    std::vector<cv::Rect>                bboxes;

    auto &gray = scene.view.gray().input();
    core->detectRegions(gray, areas, bboxes);
    
    for (size_t i = 0; i < areas.size(); ++i) {
        if ((filter == nullptr) || (filter(gray, bboxes[i], areas[i])) ) {
            Zone z(std::move(bboxes[i]), std::move(areas[i]));
            scene.mark(std::move(z)).context = Prediction(1.0f, 0, 32767);
        }
    }
   
    return Error::NONE;
}

}  // namespace Task
}  // namespace VPP
