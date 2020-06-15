/**
 *
 * @file      dscribe/pipeline.hpp
 *
 * @brief     This is the top-level d-scribe pipeline description
 *
 * @details   This file describes the description engine aimed at defining a set
 *            of multi-level visual pipelines. It is composed of a primary
 *            freezable pipeline aimed at capturing and processing images at a
 *            scene as well as some capacities to make deferred processing on
 *            dedicated zone-level pipelines.
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

#include <string>

#include "customisation/configuration.hpp"
#include "customisation/entity.hpp"
#include "vpp/pipeline.hpp"
#include "vpp/stage/blur.hpp"
#include "vpp/stage/clustering.hpp"
#include "vpp/stage/dnn.hpp"
#include "vpp/stage/input.hpp"
#include "vpp/stage/ocr/edging.hpp"
#include "vpp/stage/ocr/mser.hpp"
#include "vpp/stage/ocr/reader.hpp"
#include "vpp/stage/overlay.hpp"

namespace DScribe {

class Core : public Parametrisable {
    public:
        class Detection : public VPP::Pipeline::ForScene {
            public:
                Detection() noexcept;
                ~Detection() noexcept = default;

                VPP::Stage::Input<>           input;
                //VPP::Stage::Input<>           depth;
                VPP::Stage::Blur              blur;
                VPP::Stage::DNN::Detector     detector;
                VPP::Stage::Clustering        clustering;
                VPP::Stage::OCR::MSER         mser;
                VPP::Stage::OCR::Edging       edging;
                VPP::Stage::Overlay::ForScene overlay;
        };

        class Classification : public VPP::Pipeline::ForZone {
            public:
                Classification() noexcept;
                ~Classification() noexcept = default;

                VPP::Stage::Input<VPP::Zone> input;
                VPP::Stage::DNN::Classifier  classifier;
                VPP::Stage::OCR::Reader      ocr;
                VPP::Stage::Overlay::ForZone overlay;
        };

        Core() noexcept;

        /** DScribe cores cannot be copied nor moved */
        Core(const Core& other) = delete;
        Core(Core&& other) = delete;
        Core& operator=(const Core& other) = delete;
        Core& operator=(Core&& other) = delete;
        virtual ~Core() noexcept = default;

        /* Must be first in the list */
        Customisation::Configuration configuration;

        /* The two pipelines */
        Detection                    detection;
        Classification               classification;
};

} // namespace DScribe
