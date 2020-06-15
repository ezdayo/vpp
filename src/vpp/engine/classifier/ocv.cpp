/**
 *
 * @file      vpp/engine/classifier/ocv.cpp
 *
 * @brief     This is the VPP OCV DNN classifier implementation file
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
#include "vpp/engine/classifier/ocv.hpp"

namespace VPP {
namespace Engine {
namespace Classifier {

OCV::OCV() noexcept = default;
OCV::~OCV() noexcept = default;

Error::Type OCV::process(Scene &scene, Zone &zone) noexcept {
    cv::Mat  blob, area, output, predictions, indexes;
    cv::Mat  background(static_cast<cv::Size>(size), CV_8UC3, offset);
    const cv::Mat &input = scene.input();
    cv::Size scaled;
    cv::Point at(0,0);
    int zonew = zone.width;
    int zoneh = zone.height;
    int centrex = zone.x+zone.width/2;
    int centrey = zone.y+zone.height/2;
    float xscale, yscale, scale;
    int inputw, inputh, origw, origh;

    // Get the scaling factor (no more than 4 !)
    xscale = (static_cast<float>(size.width))/(static_cast<float>(zonew));
    yscale = (static_cast<float>(size.height))/(static_cast<float>(zoneh));
    scale = std::min(std::min(xscale, yscale), 4.0f);
   
    // Compute the actual BB size for classification 
    inputw = static_cast<int>(static_cast<float>(size.width)/scale);
    inputh = static_cast<int>(static_cast<float>(size.height)/scale);
    origw=std::min(std::min(inputw/2, centrex), input.cols-centrex);
    origh=std::min(std::min(inputh/2, centrey), input.rows-centrey);

    scaled.width = static_cast<int>(scale*2*origw);
    scaled.height = static_cast<int>(scale*2*origh);
    at.x=(static_cast<int>(size.width)-scaled.width)/2;
    at.y=(static_cast<int>(size.height)-scaled.height)/2;
  
    // Crop the region of interest
    cv::Rect ROI(centrex-origw, centrey-origh, 2*origw, 2*origh);
    cv::resize(cv::Mat(input, ROI), area, scaled, 0, 0, cv::INTER_NEAREST);
    area.copyTo(background(cv::Rect(at, scaled)));

    // Create the 4D blob corresponding to the cropped image 
    cv::dnn::blobFromImage(background, blob, scale, size, offset, RGB,
                           false);

    // Place the image-based blob at the input of the network
    net.setInput(blob);

    // Infer !
    output = net.forward();

    // Get the classes with the highest scores
    predictions = output.reshape(1, 1); 
    cv::sortIdx(predictions, indexes, cv::SORT_EVERY_ROW | cv::SORT_DESCENDING);

    for (int idx=0; idx < 5; idx++) {
        auto cid = static_cast<int16_t>(indexes.at<int>(idx));
        auto score = predictions.at<float>(cid);
        if (score > threshold) {
            zone.predictions.emplace_back(Prediction(score, dataset.ID(), cid));
        }
    }

    auto desc = label(zone);
    if (!desc.empty()) {
        zone.description += "(";
        zone.description += desc;
        zone.description += ")";
    }

    return Error::NONE;
}

}  // namespace Classifier
}  // namespace Engine
}  // namespace VPP
