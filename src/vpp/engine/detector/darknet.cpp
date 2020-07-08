/**
 *
 * @file      vpp/engine/detector/darknet.cpp
 *
 * @brief     This is the VPP Darknet detector implementation file
 *
 * @details   This engine is a Darknet YOLO detector description capable of
 *            running any YOLO DNN
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
#include "vpp/engine/detector/darknet.hpp"

/* This is missing from the standard YOLO source code includes */
extern "C" {
void fill_image(image m, float s);
void letterbox_image_into(image im, int w, int h, image boxed);
void set_batch_network(network *net, int b);
}

namespace VPP {
namespace Engine {
namespace Detector {

Darknet::Darknet() noexcept 
    : VPP::DNN::Engine::ForScene(), hierarchy(0.4), nms(0.4),
      architecture(""), weights(""), net(nullptr), input_w(-1), input_h(-1), 
      input_f(), input_p(), img_input({ }), img_yolo({ }) {
    hierarchy.denominate("hierarchy")
             .describe("The minimal YOLO hierarchy threshold")
             .characterise(Customisation::Trait::SETTABLE);
    hierarchy.range(0.0f, 1.0f);
    Customisation::Entity::expose(hierarchy);
    
    nms.denominate("nms")
       .describe("The minimal threshold to perform NMS (-1 to disable)")
       .characterise(Customisation::Trait::SETTABLE);
    nms.range(-1.0f, 1.0f);
    Customisation::Entity::expose(nms);
}

Darknet::~Darknet() noexcept = default;

Customisation::Error Darknet::setup() noexcept {
    srand(2222222);

    if ( (architecture != static_cast<std::string>(network.architecture)) || 
         (weights != static_cast<std::string>(network.weights)) ) {
        terminate();

        net = load_network_custom(const_cast<char *>(static_cast<std::string>(
                                  network.architecture).c_str()),
                                  const_cast<char *>(static_cast<std::string>(
                                  network.weights).c_str()), 1, 1);

        if (net == nullptr) {
            LOGE("%s[%s]::setup(): Cannot load Darknet DNN with config '%s' "
                 "and weights '%s'",
                 value_to_string().c_str(), name().c_str(),
                 static_cast<std::string>(network.architecture).c_str(),
                 static_cast<std::string>(network.weights).c_str());
            return Customisation::Error::INVALID_VALUE;
        }

        fuse_conv_batchnorm(*net);
        calculate_binary_weights(*net);

        architecture = network.architecture;
        weights      = network.weights;

        // Do this preventively rather than in the first inference
        set_batch_network(net, 1);
        img_yolo = make_image(net->w, net->h, 3);
    }

    /* Always restart from a default image */    
    fill_image(img_yolo, .5);

    return Customisation::Error::NONE;
}

Error::Type Darknet::process(Scene &scene) noexcept {
    int            nboxes = 0;
    const cv::Mat &input = scene.view.bgr().input();

    /* Prepare the input buffer if not ready or inappropriate */
    if ((input_w != input.cols) || (input_h != input.rows)) {
        input_w = input.cols;
        input_h = input.rows;
        img_input = make_image(input_w, input_h, 3);
        for (int p=0; p < 3; ++p) {
            input_p[p] = cv::Mat(input_h, input_w, CV_32FC1,
                                 &img_input.data[(2-p)*input_h*input_w]);
        }
    }

    // Convert the 8-bit unsigned image to 32-bit floating points
    input.convertTo(input_f, CV_32FC3, 1.0/255.0, 0);

    // Extract R, G and B planes and swap R and B
    cv::split(input_f, input_p);
    letterbox_image_into(img_input, net->w, net->h, img_yolo);

    // Infer!
    network_predict_ptr(net, img_yolo.data);
    
    // Get the boxes
    detection *dets = get_network_boxes(net, input_w, input_h,
                                        static_cast<float>(threshold),
                                        static_cast<float>(hierarchy),
                                        0, 1, &nboxes, 1);

    // Filter out the boxes
    if (static_cast<float>(nms) >= 0) {
        do_nms_obj(dets, nboxes, dataset.size(), static_cast<float>(nms));
    }

    // Capture them on the scene
    for (int i = 0; i < nboxes; ++i) {
        std::list<Prediction> predictions;

        for (int j = 0; j < (int) dataset.size(); j++) {
            auto cur_thres = dets[i].prob[j];
            if (cur_thres >= static_cast<float>(threshold)) {
                predictions.emplace_back(Prediction(cur_thres, dataset.ID(),
                                         j));
            }
        }
        
        if (predictions.empty())
            continue;
        
        box b = dets[i].bbox;
        auto &zone = scene.mark(cv::Rect_<float>(b.x-b.w/2.l, b.y-b.h/2.l,
                                b.w, b.h)).predict(predictions);
        zone.description = label(zone);
    }
    
    free_detections(dets, nboxes);

    return Error::NONE;
}

void Darknet::terminate() noexcept {
    if (net != nullptr) {
        architecture.clear();
        weights.clear();

        free_network(*net);
        net = nullptr;

        input_f = cv::Mat();
        for (auto &p : input_p) {
            p = cv::Mat();
        }

        if  ( (input_w > 0) && (input_h > 0) ) {
            free_image(img_input);
            img_input = { };
        }
        input_w = -1;
        input_h = -1;

        free_image(img_yolo);
        img_yolo = { };
    }
}

}  // namespace Detector
}  // namespace Engine
}  // namespace VPP
