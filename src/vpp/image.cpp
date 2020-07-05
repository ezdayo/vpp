/**
 *
 * @file      vpp/image.cpp
 *
 * @brief     This is the VPP image implementation file
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
#include <unordered_map>

#include "vpp/image.hpp"

namespace VPP {

Image Image::INVALID;

Image::Image() noexcept 
    : m(), boundaries(), original(), copy() {
}

#ifdef NDEBUG
static inline void check_validity(const cv::Mat &/*data*/,
                                  const Image::Mode &/*mode*/) {}
#else
static void check_validity(const cv::Mat &data,
                           const Image::Mode &mode) {
    ASSERT(mode.channels() == data.channels(),
           "Image::Image(): Provided a %d-channel image for a %d-channel "
           "mode (0x%x)!", data.channels(), mode.channels(),
           static_cast<int>(mode));
    ASSERT(!data.empty(), "Image::Image(): Provided an empty image!");
}
#endif

Image::Image(cv::Mat data, Image::Mode mode) noexcept
    : m(std::move(mode)),
      boundaries(std::move(cv::Rect(0, 0, data.cols, data.rows))), 
      original(std::move(data)), copy() {
    check_validity(original, m);
}

Image::Image(const Image &i, Image::Mode mode, const cv::Rect &roi,
             float scale, float offset) noexcept
    : m(std::move(mode)), 
      original(std::move(i.to(m, roi, scale, offset))), copy() {
    check_validity(original, m);
    boundaries = std::move(cv::Rect(0, 0, original.cols, original.rows));
}

Image::Image(const Image &i, Image::Mode mode, float scale, float offset)
    noexcept : m(std::move(mode)), boundaries(i.boundaries),
               original(std::move(i.to(m, scale, offset))), copy() {
    check_validity(original, m);
}

Image::Image(const Image& other) noexcept 
: m(other.m), boundaries(other.boundaries), original(other.original), 
  copy() { }

Image& Image::operator=(const Image& other) noexcept {
    if (&other != this) {
        m          = other.m;
        boundaries = other.boundaries;
        original   = other.original;
        copy       = cv::Mat();
    }

    return *this;
}

Image::~Image() noexcept = default;

Image Image::operator()(const cv::Rect &roi) const noexcept {
    return Image(original(roi & boundaries), m); 
}

const cv::Mat &Image::output() const noexcept {
    if (copy.empty()) {
        return original;
    }
    return copy;
}

cv::Mat &Image::drawable() noexcept {
    if (copy.empty()) {
        flush();
    }
    return copy;
}

void Image::flush() noexcept {
    original.copyTo(copy);
}

static bool extract_is_valid(const Image::Channel &c,
                             const Image::Mode &m) noexcept {
    if (!c.in(m)) {
        LOGE("Image::extract(): Cannot extract channel %d from an image "
             "in mode %d!", static_cast<int>(c), static_cast<int>(m));
        return false;
    }

    return true;
}

cv::Mat Image::extract(const Image::Channel &c,
                       const cv::Rect &roi) const noexcept {
    cv::Mat plane;

    if (extract_is_valid(c, m)) {
        cv::extractChannel(original(roi & boundaries), plane, c.id());
    } else {
        LOGD("Image::extract(): cannot extract channel %d from an image "
             "of mode %d!", c.id(), static_cast<int>(m));
    }

    return plane;
}

cv::Mat Image::extract(const Image::Channel &c) const noexcept {
    cv::Mat plane;

    if (extract_is_valid(c, m)) {
        cv::extractChannel(original, plane, c.id());
    }
        
    return plane;
}

bool Image::translatable(const Image::Mode &mode) const noexcept {
    /* If any is not valid then we are screwed */
    if ((! mode.valid()) || (! m.valid())) {
        return false;
    }

    /* If the mode is the same, then we are fine */
    if (m == mode) {
        return true;
    }

    /* If one is depth and the other is normal image, then we are screwed */
    if (mode.is_depth() != m.is_depth()) {
        return false;
    }

    /* If both are depth images, then it is ok */
    if (m.is_depth()) {
        return true;
    }

    /* From here the two modes are valid image modes so check if at least one 
     * is BGR otherwise we cannot translate directly and we'll need an
     * intermediate BGR matrix */
    return (mode == Mode::BGR) || (m == Mode::BGR);
}

cv::Mat Image::to(const Image::Mode &mode, const cv::Rect &roi, float scale,
                  float offset) const noexcept {
    cv::Mat out;

    /* Shall never call a translation if it is not possible */
    ASSERT(translatable(mode),
           "Image::to(): Cannot translate an image of type %d to an image "
           "of type %d!", static_cast<int>(m), static_cast<int>(mode));
    
    if (!translatable(mode)) {
        return out;
    }

    /* Do the cropping carefully not to go beyond the limits */
    cv::Mat in(std::move(original(roi & boundaries)));

    if (m == mode) {
        return out;
    }

    /* From this point, the two modes are either both depth modes or standard
     * image modes */

    /* If depth mode, then change type and scale */
    if (m.is_depth()) {
        if (mode == Mode::DEPTHF) {
            in.convertTo(out, CV_32F, scale, offset);
        } else {
            in.convertTo(out, CV_16U, scale, offset);
        }
        return out;
    }

    /* The two modes are standard image modes, and hence, either the source
     * or the destination is BGR */
    int conversion;
    if (m == Mode::BGR) {
        switch(mode) {
            case Mode::HSV:
                conversion=cv::COLOR_BGR2HSV;
                break;
            case Mode::YUV:
                conversion=cv::COLOR_BGR2YUV;
                break;
            case Mode::YCrCb:
                conversion=cv::COLOR_BGR2YCrCb;
                break;
            case Mode::GRAY:
                conversion=cv::COLOR_BGR2GRAY;
                break;
            default:
                ASSERT(false, "Image::to(): Cannot convert from BGR. "
                              "This shall never happen!");
                return out;
        }
    } else {
        switch(m) {
            case Mode::HSV:
                conversion=cv::COLOR_HSV2BGR;
                break;
            case Mode::YUV:
                conversion=cv::COLOR_YUV2BGR;
                break;
            case Mode::YCrCb:
                conversion=cv::COLOR_YCrCb2BGR;
                break;
            case Mode::GRAY:
                conversion=cv::COLOR_GRAY2BGR;
                break;
            default:
                ASSERT(false, "Image::to(): Cannot convert to BGR. "
                              "This shall never happen!");
                return out;
        }
    }
   
    /* Perform the right colour conversion */
    cv::cvtColor(in, out, conversion, 0);

    return out;
}

cv::Mat Image::to(const Image::Mode &mode, float scale,
                  float offset) const noexcept {
    return to(mode, boundaries, scale, offset);
}

}  // namespace VPP
