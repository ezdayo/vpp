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

#include <fstream>
#include <memory>
#ifdef VPP_HAS_EXTERNAL_FONT_SUPPORT
#include <opencv2/freetype.hpp>
#endif
#ifdef VPP_HAS_IMAGE_CODEC_SUPPORT
#include <opencv2/imgcodecs.hpp>
#endif
#include <opencv2/imgproc.hpp>
#include <sstream>
#include <unordered_map>

#include "vpp/log.hpp"
#include "vpp/util/ocv/overlay.hpp"
#include "vpp/util/utf8.hpp"

namespace Util {
namespace OCV {

namespace Font {

    /* Valid internal Hershey Fonts */
    static std::unordered_map<std::string,
                              enum cv::HersheyFonts> valid_internals = {
    { "HERSHEY_SIMPLEX",        cv::FONT_HERSHEY_SIMPLEX },
    { "HERSHEY_PLAIN",          cv::FONT_HERSHEY_PLAIN },
    { "HERSHEY_DUPLEX",         cv::FONT_HERSHEY_DUPLEX },
    { "HERSHEY_COMPLEX",        cv::FONT_HERSHEY_COMPLEX },
    { "HERSHEY_TRIPLEX",        cv::FONT_HERSHEY_TRIPLEX },
    { "HERSHEY_COMPLEX_SMALL",  cv::FONT_HERSHEY_COMPLEX_SMALL },
    { "HERSHEY_SCRIPT_SIMPLEX", cv::FONT_HERSHEY_SCRIPT_SIMPLEX },
    { "HERSHEY_SCRIPT_COMPLEX", cv::FONT_HERSHEY_SCRIPT_COMPLEX },
    { "ITALIC",                 cv::FONT_ITALIC } };

    class Internal : public Overlay::Font {
        public:
            explicit Internal(std::string name, std::string path,
                              int id) noexcept : 
                Overlay::Font(name, path),
                font(static_cast<enum cv::HersheyFonts >(id)) {
                ASSERT(id >= 0, "Invalid Hershey Font id provided : %d", id);
            }

            virtual ~Internal() noexcept = default;

            static int valid(const std::string &path) noexcept {
                static const std::string italix("ITALIC");
                std::string              key(path);
                int                      id(0);

                /* If the path ends with "_ITALIC" then keep the italic mask */
                if ( (path.size() > (italix.size() + 1)) && 
                     (path.compare(path.size()-italix.size(), italix.size(),
                                   italix) == 0) ) {
                    id = valid_internals[italix];
                    key.resize(path.size()-italix.size()-1);
                }

                /* Try to find the required font and return the font id if
                 * found or -1 if not found */
                auto found = valid_internals.find(key);
                if (found != valid_internals.end()) {
                    return id + found->second;
                } else {
                    return -1;
                }
            };

            /* Drawing some text */
            virtual void write(cv::Mat &frame, const std::string &text,
                               const cv::Point &at, int thickness,
                               Overlay::AAMode antialiasing, cv::Scalar color,
                               int height) const noexcept {
                /* Always use a "coherent" thickness */
                if (thickness <= 0) {
                    return;
                }

                std::istringstream input(text);
                auto start = input.tellg();
                std::string line;
                cv::Size maxSz, curSz;
                int lineCnt = 0;
                int baseLine;
                auto fontScale = static_cast<double>(height)/32.0;

                while (std::getline(input, line)) {
                    ++lineCnt;
                    /* Default fonts are not accentuented, hence remove the 
                     * accents */
                    Util::UTF8::toASCII(line);
                    curSz = cv::getTextSize(line, font, fontScale, thickness,
                                            &baseLine);
                    if (curSz.width > maxSz.width) maxSz = curSz;
                }

                cv::Point offset = cv::Point(-maxSz.width, -lineCnt*height);

                input.clear();
                input.seekg(start, input.beg);
                lineCnt = 0;
                while (std::getline(input, line)) {
                    Util::UTF8::toASCII(line);
                    cv::putText(frame, line, 
                                at + offset/2 + cv::Point(0,(lineCnt+1)*height),
                                font, fontScale, color, thickness, 
                                static_cast<int>(antialiasing), false);
                    ++lineCnt;
                }
            }
            
        private:
            enum cv::HersheyFonts font;
    };

#ifdef VPP_HAS_EXTERNAL_FONT_SUPPORT
    class TTF : public Overlay::Font {
        public:
            explicit TTF(std::string name, std::string path) noexcept :
                Overlay::Font(name, path) {
                font = cv::freetype::createFreeType2();
                font->loadFontData(path, 0);
            }

            virtual ~TTF() noexcept = default;

            /* Drawing some text */
            virtual void write(cv::Mat &frame, const std::string &text,
                               const cv::Point &at, int thickness,
                               Overlay::AAMode antialiasing, cv::Scalar color,
                               int height) const noexcept {
                /* Always use a "coherent" thickness */
                if (thickness <= 0) {
                    return;
                }

                std::istringstream input(text);
                auto start = input.tellg();
                std::string line;
                cv::Size maxSz, curSz;
                int lineCnt = 0;
                int baseLine;
    
                while (std::getline(input, line)) {
                    ++lineCnt;
                    curSz = font->getTextSize(line, height, -1, &baseLine);
                    if (curSz.width > maxSz.width) maxSz = curSz;
                }

                cv::Point offset = cv::Point(-maxSz.width, -lineCnt*height);

                input.clear();
                input.seekg(start, input.beg);
                lineCnt = 0;
                while (std::getline(input, line)) {

                    font->putText(frame, line, 
                                  at + offset/2 +
                                  cv::Point(0, ((4*lineCnt-1)*height)/4),
                                  height, color, -1, 
                                  static_cast<int>(antialiasing), false);
                    ++lineCnt;
                }
            }
            
        private:
            cv::Ptr<cv::freetype::FreeType2> font;
    };
#endif

}  // namespace Font

/* Static dictionnary for all defined fonts */
static std::unordered_map<std::string,
                          std::unique_ptr<Overlay::Font>> defined_fonts;

Overlay::Font::Font(std::string id, std::string path) noexcept
    : name(std::move(id)), location(std::move(path)) {}
Overlay::Font::~Font() noexcept = default;

Overlay::Font *Overlay::Font::any() noexcept {
    return use("HERSHEY_SIMPLEX");
}

Overlay::Font *Overlay::Font::use(const std::string &name) noexcept {
    auto found = defined_fonts.find(name);

    /* If the requested font is found, then use it */
    if (found != defined_fonts.end()) {
        return found->second.get();
    }

    /* Otherwise get the default Hershey Simplex font */
    found = defined_fonts.find("HERSHEY_SIMPLEX");
    if (found != defined_fonts.end()) {
        return found->second.get();
    }

    /* Otherwise create and use the default Hershey Simplex font */
    return use("HERSHEY_SIMPLEX", "HERSHEY_SIMPLEX");
}

Overlay::Font *Overlay::Font::use(const std::string &name,
                                  const std::string &path) noexcept {
    auto found = defined_fonts.find(name);

    /* If the requested font is found, then use it if it has the same path */
    if (found != defined_fonts.end()) {
        if (found->second->location == name) {
            LOGW("Overlay::Font::use(name, path): Redefining font %s with a "
                 "different font path %s! Keeping the initial one at %s.",
                 name.c_str(), path.c_str(), found->second->location.c_str());
        }    
        return found->second.get();
    }

    /* Create an internal font if it is an internal font path */
    auto id = Util::OCV::Font::Internal::valid(path);
    if (id >= 0) {
        defined_fonts.emplace(name, 
                              std::move(std::unique_ptr<Overlay::Font>
                              (new Util::OCV::Font::Internal(name, path, id))));
        return use(name);
    }

#ifdef VPP_HAS_EXTERNAL_FONT_SUPPORT
    /* Otherwise try to create a freetype font as it was not an internal path */
    std::ifstream ifs(path);
    if (ifs.is_open()) {
        defined_fonts.emplace(name, 
                              std::move(std::unique_ptr<Overlay::Font>
                              (new Util::OCV::Font::TTF(name, path))));
        return use(name);
    }
#endif

    /* Otherwise get the default Hershey Simplex font in last resort */
    return use("HERSHEY_SIMPLEX", "HERSHEY_SIMPLEX");
}

void Overlay::Font::write(cv::Mat &/*frame*/, const std::string &/*text*/,
                          const cv::Point &/*at*/, int /*thickness*/,
                          AAMode /*antialiasing*/, cv::Scalar /*color*/,
                          int /*height*/) const noexcept {
    LOGE("Overlay::Font::write() has to be implemented in all child classes.");
}

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
#ifdef VPP_HAS_IMAGE_CODEC_SUPPORT
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
#else
    LOGW("Overlay::Layer::set(filename): Cannot read %s since there is no such "
         "support provided by OpenCV. Rebuild the VPP with support for OpenCV "
         "imgproc.", filename.c_str());
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
    defaultTextStyle.font         = nullptr;
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
    auto font = style.font;
    if (font == nullptr) {
        font = Font::any(); 
    }
    font->write(frame, text, at, style.thickness, style.antialiasing,
                style.color, style.height);
}

}  // namespace OCV
}  // namespace Util
