/**
 *
 * @file      vpp/util/ocv/overlay.cpp
 *
 * @brief     This is a versatile OpenCV overlay class
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

#include "vpp/config.hpp"

#ifdef OPENCV_IMGCODECS_FOUND
#include <opencv2/imgcodecs.hpp>
#endif
#include <sstream>

#include "vpp/log.hpp"
#include "vpp/util/ocv/overlay.hpp"
#include "vpp/util/utf8.hpp"

namespace Util {
namespace OCV {

#ifndef USE_SPECIAL_FONT
const static auto STANDARD_FONT(cv::FONT_HERSHEY_SIMPLEX);
#endif

Overlay::Layer::Layer() noexcept : width(0), height(0), fg(), msk() {}
Overlay::Layer::~Layer() noexcept = default;

void Overlay::Layer::clear() noexcept {
    fg     = cv::Mat();
    msk    = cv::Mat();
    width  = 0;
    height = 0;
}

bool Overlay::Layer::empty() const noexcept {
    return ((width <= 0) || (height <= 0));
}

void Overlay::Layer::merge(cv::Mat &frame, const cv::Point &at,
                           const Overlay::LayerStyle &style) const noexcept {
    
    /* We're done if there is nothing to merge from */
    if (empty()) {
        return;
    }

    cv::Mat bg;

    cv::Rect location(at, fg.size());
    if (location.x < 0) {
        location.x += frame.cols-fg.cols;
    }

    if (location.y < 0) {
        location.y += frame.rows-fg.rows;
    }

    cv::Mat(frame, location).convertTo(bg, CV_32FC3, 1.0, 0);
    bg = bg.mul(msk, style.saturation);
    cv::add(fg, bg, frame(location), cv::noArray(), -1);
}

void Overlay::Layer::set(const std::string &filename) noexcept {
#ifdef OPENCV_IMGCODECS_FOUND
    if (filename.empty()) {
        clear();
        return;
    }

    auto bgra = cv::imread(filename.c_str(), cv::IMREAD_UNCHANGED);

    /* Safety checks */
    if ((bgra.data == nullptr) || (bgra.channels() != 4)) {
        clear();
        return;
    }

    cv::Mat chans[4], bgr;
    cv::split(bgra, chans);
    cv::merge(chans, 3, bgr);
    set(bgr, chans[3]);
#endif
}

void Overlay::Layer::set(cv::Size size, const uint8_t *bgr, 
                         const uint8_t *alpha) noexcept {
    if ((size.width <= 0) || (size.height <= 0)) {
        return clear();
    }

    set(cv::Mat(size, CV_8UC(3),
                static_cast<void *>(const_cast<uint8_t*>(bgr)), 
                cv::Mat::AUTO_STEP),
        cv::Mat(std::move(size), CV_8UC1, 
                static_cast<void *>(const_cast<uint8_t*>(alpha)),
                cv::Mat::AUTO_STEP));
}

void Overlay::Layer::set(const cv::Mat &bgr, const cv::Mat &alpha) noexcept {
    ASSERT((bgr.size() == alpha.size()),
           "Overlay::Layer::set(): Invalid bgr and alpha channels provided!");
    
    if ((bgr.empty()) || (alpha.empty())) {
        return clear();
    }

    cv::Mat msk1, msk3;

    alpha.convertTo(msk1, CV_32FC1, 1.0/255.0, 0);
    cv::cvtColor(msk1, msk3, cv::COLOR_GRAY2RGB, 3);
    bgr.convertTo(fg, CV_32FC3, 1.0, 0);
    fg = fg.mul(msk3, 1.0);
    cv::cvtColor(1.0 - msk1, msk, cv::COLOR_GRAY2RGB, 3);

    width  = fg.cols;
    height = fg.rows; 
}

Overlay::Overlay() noexcept {
    resetDefaultDrawingStyle();
    resetDefaultLayerStyle();
    resetDefaultTextStyle();

#ifdef USE_SPECIAL_FONT
    FT2 = cv::freetype::createFreeType2();
    FT2->loadFontData(USE_SPECIAL_FONT, 0);
#endif
}

Overlay::~Overlay() noexcept = default;

void Overlay::resetDefaultDrawingStyle() noexcept {
    defaultDrawingStyle.thickness    = 2;
    defaultDrawingStyle.antialiasing = AAMode::LINE_8;
    defaultDrawingStyle.color        = { 0, 255, 0 };
}

void Overlay::resetDefaultLayerStyle() noexcept {
    defaultLayerStyle.saturation    = 1.0;
}

void Overlay::resetDefaultTextStyle() noexcept {
    defaultTextStyle.thickness    = 2;
    defaultTextStyle.antialiasing = AAMode::LINE_8;
    defaultTextStyle.color        = { 0, 255, 0 };
    defaultTextStyle.height       = 32;
}
        
void Overlay::draw(cv::Mat &frame, const cv::Size &box) const noexcept {
    /* Center the box by default */
    draw(frame, box, cv::Point((frame.cols - box.width)/2,
                               (frame.rows - box.height)/2));
}
        
void Overlay::draw(cv::Mat &frame, const cv::Size &box, const cv::Point &at)
    const noexcept {
        draw(frame, box, at, defaultDrawingStyle);
}

void Overlay::draw(cv::Mat &frame, const cv::Size &box, const cv::Point &at,
                   const DrawingStyle &style) const noexcept {

    cv::Rect rect(at, box);
    if (rect.x < 0) {
        rect.x += frame.cols-box.width;
    }

    if (rect.y < 0) {
        rect.y += frame.rows-box.height;
    }

    draw(frame, rect, style);
}

void Overlay::draw(cv::Mat &frame, const cv::Rect &box) const noexcept {
    draw(frame, box, defaultDrawingStyle);
}

void Overlay::draw(cv::Mat &frame, const cv::Rect &box,
                   const DrawingStyle &style) const noexcept {

    auto thickness = style.thickness;
    if ((thickness <= 0) && (style.color[3] > 0)) {
        double alpha = static_cast<double>(style.color[3])/255.0;
        auto bg = frame(box);
        auto fg = cv::Mat(box.size(), CV_8UC3, style.color);
        cv::addWeighted(bg, alpha, fg, 1.0-alpha, 0.0, bg);

        thickness = -thickness;
    }

    /* Skip drawing a rectangle altogether if the thickness is 0 */
    if (thickness != 0) {
        cv::rectangle(frame, box, style.color, thickness,
                      static_cast<int>(style.antialiasing), 0);
    }
}

void Overlay::draw(cv::Mat &frame, const Overlay::Layer &layer) const noexcept {
    /* Center the layer by default */
    return draw(frame, layer, cv::Point((frame.cols - layer.width)/2,
                                        (frame.rows - layer.height)/2));
}

void Overlay::draw(cv::Mat &frame, const Overlay::Layer &layer, 
                   const cv::Point &at) const noexcept {
    return draw(frame, layer, at, defaultLayerStyle);
}

void Overlay::draw(cv::Mat &frame, const Overlay::Layer &layer, 
                   const cv::Point &at, const LayerStyle &style) 
    const noexcept {
    return layer.merge(frame, at, style);
}

void Overlay::draw(cv::Mat &frame, const std::string &text) const noexcept {
    /* Center the text by default */
    return draw(frame, text, cv::Point(frame.cols/2, frame.rows/2));
}

void Overlay::draw(cv::Mat &frame, const std::string &text, const cv::Point &at)
    const noexcept {
    return draw(frame, text, at, defaultTextStyle);
}

void Overlay::draw(cv::Mat &frame, const std::string &text, const cv::Point &at,
                   const TextStyle &style) const noexcept {
        
    /* Always use a "coherent" thickness */
    if (style.thickness <= 0) {
        return;
    }

    std::istringstream input(text);
    auto start = input.tellg();
    std::string line;
    cv::Size maxSz, curSz;
    int lineCnt = 0;
    int baseLine;
#ifndef USE_SPECIAL_FONT
    auto fontScale = static_cast<double>(style.height)/32.0;
#endif

    while (std::getline(input, line)) {
        ++lineCnt;
#ifdef USE_SPECIAL_FONT
        curSz = FT2->getTextSize(line, style.height, -1, &baseLine);
#else
        /* Default fonts are not accentuented, hence remove the accents */
        Utils::UTF8::toASCII(line);
        curSz = cv::getTextSize(line, STANDARD_FONT, fontScale, style.thickness,
                                &baseLine);
#endif
        if (curSz.width > maxSz.width) maxSz = curSz;
    }

    cv::Point offset = cv::Point(-maxSz.width, -lineCnt*style.height);

    input.clear();
    input.seekg(start, input.beg);
    lineCnt = 0;
    while (std::getline(input, line)) {

#ifdef USE_SPECIAL_FONT
        FT2->putText(frame, line, 
                     at + offset/2 +
                     cv::Point(0, ((4*lineCnt-1)*style.height)/4),
                     style.height, style.color, -1, 
                    static_cast<int>(style.antialiasing), false);
#else
        Utils::UTF8::toASCII(line);
        cv::putText(frame, line, 
                    at + offset/2 + cv::Point(0, (lineCnt+1)*style.height),
                    STANDARD_FONT, fontScale, style.color, style.thickness, 
                    static_cast<int>(style.antialiasing), false);
#endif
        ++lineCnt;
    }
}

}  // namespace OCV
}  // namespace Util
