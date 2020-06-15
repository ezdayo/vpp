/**
 *
 * @file      vpp/engine/detector/ocv.cpp
 *
 * @brief     This is the VPP OCV DNN detector implementation file
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

#include <opencv2/opencv.hpp>

#include "vpp/log.hpp"
#include "vpp/engine/detector/ocv.hpp"

namespace VPP {
namespace Engine {
namespace Detector {

OCV::OCV() noexcept 
    : VPP::DNN::Engine::OCV<>(), nms(0.4), names(), outLayers(),
      outLayerType(""), needsResizing(false), imInfo() {
    nms.denominate("nms")
       .describe("The minimal threshold to perform NMS (-1 to disable)")
       .characterise(Customisation::Trait::SETTABLE);
    nms.range(-1.0f, 1.0f);
    Customisation::Entity::expose(nms);
}

OCV::~OCV() noexcept = default;

Customisation::Error OCV::setup() noexcept {
    auto error = VPP::DNN::Engine::OCV<>::setup();

    if (error == Customisation::Error::NONE) {
        // If the network misses im_info (Faster-RCNN & R-FCN), then recreate it
        needsResizing=(net.getLayer(0)->outputNameToIndex("im_info") != -1);
        if (needsResizing) {
            imInfo = (cv::Mat_<float>(1, 3) << static_cast<int>(size.height),
                                               static_cast<int>(size.width),
                                               1.6f);
        }

        // Store the network output names
        outLayers = net.getUnconnectedOutLayers();
        outLayerType = net.getLayer(outLayers[0])->type;
        std::vector<cv::String> layersNames = net.getLayerNames();
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }

    return error;
}

Error::Type OCV::process(Scene &scene) noexcept {
    cv::Mat              blob;
    std::vector<cv::Mat> outputs;
    const cv::Mat &      input = scene.input();

    // Create the 4D blob corresponding to the input image without cropping it 
    cv::dnn::blobFromImage(input, blob, scale, static_cast<cv::Size>(size), 
                           offset, static_cast<bool>(RGB), false);

    // Place the image-based blob at the input of the network
    net.setInput(blob);

    // Resize the input if it needs to be resized
    if (needsResizing) {
        cv::resize(input, input, static_cast<cv::Size>(size));
        net.setInput(imInfo, "im_info");
    }

    // Run the network
    net.forward(outputs, names);

    // Extract and display the information
    if ( (needsResizing) || (outLayerType == "DetectionOutput") ) {
        ASSERT(outputs.size() == 1, "%s[%s]::process(): "
               "Expecting a single output OCV DNN!",
               value_to_string().c_str(), name().c_str());
        float* data = (float*) outputs[0].data;
        for (size_t i = 0; i < outputs[0].total(); i += 7)
        {
            float confidence = data[i + 2];
            if (confidence > threshold)
            {
                int classId = (int)(data[i + 1]) - 1; 
                auto &zone = scene.mark(cv::Rect_<float>(data[i+3], data[i+4],
                                                         data[i+5], data[i+6]));
                zone.predict(Prediction(confidence, dataset.ID(), classId));
                zone.description = label(zone);
            }
        }
    } else if (outLayerType == "Region") {
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        for (size_t i = 0; i < outputs.size(); ++i)
        {
            float* data = (float*)outputs[i].data;
            for (int j = 0; j < outputs[i].rows; ++j, data += outputs[i].cols)
            {
                cv::Mat scores = outputs[i].row(j).colRange(5, outputs[i].cols);
                cv::Point classIdPoint;
                double confidence;
                cv::minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                if (confidence > threshold)
                {
                    int centerX = (int)(data[0] * input.cols);
                    int centerY = (int)(data[1] * input.rows);
                    int width = (int)(data[2] * input.cols);
                    int height = (int)(data[3] * input.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }

        if (static_cast<float>(nms) >= 0) {
            std::vector<int> indices;
            cv::dnn::NMSBoxes(boxes, confidences, threshold, nms, indices);
            for (size_t i = 0; i < indices.size(); ++i) {
                int idx = indices[i];
                cv::Rect box = boxes[idx];
                
                auto &zone = 
                    scene.mark(box).predict(Prediction(confidences[idx], 
                                                       dataset.ID(), 
                                                       classIds[idx]));
                zone.description = label(zone);
            }
        } else {
            for (size_t idx = 0; idx < boxes.size(); ++idx) {
                cv::Rect box = boxes[idx];
                
                auto &zone = 
                    scene.mark(box).predict(Prediction(confidences[idx], 
                                                       dataset.ID(), 
                                                       classIds[idx]));
                zone.description = label(zone);
            }
        }
    } else {
        LOGE("%s[%s]::process(): Unknown output layer type '%s'",
             value_to_string().c_str(), name().c_str(), outLayerType.c_str());
        return Error::NOT_EXISTING;
    }   

    return Error::NONE;
}

void OCV::terminate() noexcept {
    VPP::DNN::Engine::OCV<>::terminate();
    names.clear();
    outLayers.clear();
    outLayerType = "";
    needsResizing = false;
    imInfo = cv::Mat();
}

}  // namespace Detector
}  // namespace Engine
}  // namespace VPP
