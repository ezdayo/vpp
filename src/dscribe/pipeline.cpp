/**
 *
 * @file      dscribe/pipeline.cpp
 *
 * @brief     This is the top-level d-scribe pipeline implementation
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

#include "dscribe/pipeline.hpp"

/* Macro for exposing and naming a sub-entity */
#define USES(x) expose(x.denominate(#x))

namespace DScribe {

static void track_scene(const VPP::Scene &s, int error) noexcept {
    if (error == Error::NONE) {
        static VPP::Scene history;
        history = std::move(const_cast<VPP::Scene &>(s).update(history));
    }
}

Core::Detection::Detection() noexcept
    : VPP::Pipeline::ForScene(), input(), /*depth(),*/ blur(), detector(), 
      clustering(), overlay() {
    USES(input);
    /*USES(depth);*/
    USES(blur);
    USES(detector);
    USES(clustering);
    USES(mser);
    USES(edging);
    USES(overlay);

    input.use("capture");

    VPP::Tracker::State::DEFAULT.predictability(2);
    /*detector.broadcast.connect(track_scene);*/

    /* Only cluster if there are text zone, and only cluster those text zones */
    clustering.filter = ([](const VPP::Scene &s) noexcept {
                            for (auto &z : s.zones()) {
                                if (VPP::DNN::Dataset::isText(z)) {
                                    return true;
                                }
                            } 
                            return false; });
    clustering.basic.dnj.filter = VPP::DNN::Dataset::isText;
    clustering.basic.similarity.filter = VPP::DNN::Dataset::isText;

    /* Create the pipeline! */
    *this >> input >> /*depth >>*/ blur >> detector >> clustering >> mser 
          >> edging >> overlay;
}

Core::Classification::Classification() noexcept
    : VPP::Pipeline::ForZone(), input(), classifier(), ocr(), overlay() {
    USES(input);
    USES(classifier);
    USES(ocr);
    USES(overlay);

    input.use("bridge");

    /* Create the pipeline! */
    *this >> input >> ocr >> classifier >> overlay;
}

Core::Core() noexcept
    : Customisation::Entity("DScribe"), configuration(), detection(), 
      classification() {
    USES(configuration);
    USES(detection);
    USES(classification);

    /* Forward the detected scene to the classification pipeline */
    detection.finished = 
        [this] (VPP::Scene &s) {
            if (!s.zones().empty()) {
                classification.input.bridge.forward(std::move(s));
                classification.input.bridge.forward(
                    std::move(classification.input.bridge.scene().zones()));

            } };

    denominate("dscribe");
}

}  // namespace DScribe
