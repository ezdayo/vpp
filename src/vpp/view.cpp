/**
 *
 * @file      vpp/view.cpp
 *
 * @brief     This is the VPP view implementation file
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#include <chrono>
#include <ctime>
#include <vector>

#include "vpp/log.hpp"
#include "vpp/view.hpp"

namespace VPP {

Error::Type View::Depth::map(const Image &d, const Projecter &p) noexcept {

    /* If a map is already set, then this is a remap! */
    if (projecter != nullptr) {
        ASSERT(projecter == &p, 
               "View::Depth::map(): Cannot map an already mapped depth object"
               "with a different projecter!");
        return remap(d);
    }

    ASSERT( (d.mode() == Image::Mode::DEPTH16) ||
            (d.mode() == Image::Mode::DEPTHF),
           "View::Depth::map(): Cannot map a non-depth image of mode %d "
           "as a depth-map!", static_cast<int>(d.mode()));

    depth_map = &d;
    projecter = &p;
        
    return Error::NONE;
}

Error::Type View::Depth::remap(const Image &d, bool force) noexcept {
    ASSERT( (d.mode() == Image::Mode::DEPTH16) ||
            (d.mode() == Image::Mode::DEPTHF),
           "View::Depth::remap(): Cannot remap a non-depth image of mode %d "
           "as a depth-map!", static_cast<int>(d.mode()));
    
    /* Ensure that the projecter is valid */
    ASSERT(projecter != nullptr,
          "View::Depth::remap(): Remaped whilst not having any projecter!");

    if (depth_map != nullptr) {
        /* Update the depth map only if providing a faster lookup version with
         * computed floating point data or if forcing is requested, otherwise
         * this is definitely an error that shall never happen! */
        if ( (!force) && ((d.mode()          == Image::Mode::DEPTH16) || 
                          (depth_map->mode() == Image::Mode::DEPTHF)) ) {
            ASSERT((d.mode() == depth_map->mode()) && 
                   (cv::sum(d.input() != depth_map->input()) == cv::Scalar(0)),
                   "View::Depth::remap(): Changing the depth-map with a "
                   "different one without forcing!");
            return Error::INVALID_REQUEST;
        }
    }

    depth_map = &d;
    
    return Error::NONE;
}

float View::Depth::at(const cv::Point &pix) const noexcept {
    /* If there is no depth map and no projection, or if the pixel is outside
     * of the map then there is no depth! */
    if ((depth_map == nullptr) || (!depth_map->frame().contains(pix)) ) {
       return -1;
    }

    if (depth_map->mode() == Image::Mode::DEPTHF) {
        return depth_map->input().at<float>(pix); 
    } else {
        return static_cast<float>(depth_map->input().at<uint16_t>(pix)) *
               projecter->zscale; 
    }
}

float View::Depth::at(const cv::Rect &area) const noexcept {
    /* If there is no depth map and no projection, then there is no depth! */
    if (depth_map == nullptr) {
       return -1;
    }

    const auto &map = depth_map->input()(area & depth_map->frame());
    auto mask = (map > 0);
    auto z = static_cast<float>(cv::mean(map, mask)[0]);

    if (z <= 0) {
        return -1;
    }

    if (depth_map->mode() == Image::Mode::DEPTH16) {
        return z * projecter->zscale;
    }

    return z;
}

float View::Depth::scaler(const Image::Mode &from,
                          const Image::Mode &to) const noexcept {
    /* If there is no depth map and no projection, then there is no scaler */
    if (projecter == nullptr) {
        return 0.0;
    }

    /* Otherwise provide the right scaler */
    if ( (from == Image::Mode::DEPTH16) && (to == Image::Mode::DEPTHF) ) {
        return projecter->zscale;
    }
    if ( (from == Image::Mode::DEPTHF) && (to == Image::Mode::DEPTH16) ) {
        return 1.0f/projecter->zscale;
    }

    return 1.0;
}

cv::Point3f View::Depth::deproject(const cv::Point &p, float z) const noexcept {
    /* If there is no depth map and no projection, or if the provided z depth
     * is not valid then projection is only about adding an invalid depth */
    if ( (projecter == nullptr) || (z <= 0) ) {
        return cv::Point3f(p.x, p.y, -1);
    }
    
    return projecter->deproject(p, z);
}

cv::Point3f View::Depth::deproject(const cv::Point &p,
                                   const std::vector<uint16_t> &neighbours)
    const noexcept {
    /* If there is no depth map and no projection, then deprojection is only
     * adding a nil depth to the provided point */
    if (depth_map == nullptr) {
        return cv::Point3f(p.x, p.y, -1);
    }

    /* If the requested neighbours are empty then use a valid one */
    if (neighbours.empty()) {
        return deproject(p);
    }

    float z = -1;
    for (auto n : neighbours) {
        if (n == 0) {
            z = at(p);
        } else {
            z = at(cv::Rect(p.x-n, p.y-n, 2*n+1, 2*n+1));
        }

        /* Stop as soon as a valid depth is found */
        if (z > 0) {
            break;
        }
    }

    if (z > 0) {
        return projecter->deproject(p, z);
    }

    return cv::Point3f(p.x, p.y, -1);
}

static const std::vector<uint16_t> fallback_neighbourhood = \
    { 0, 4, 8, 16, 32, 64, 128 };

cv::Point3f View::Depth::deproject(const cv::Point &p) const noexcept {
    /* Use a valid neighbourhood for capturing the right depth */
    if (!neighbourhood.empty()) {
        return deproject(p, neighbourhood);
    }

    if (!default_neighbourhood.empty()) {
        return deproject(p, default_neighbourhood);
    }

    return deproject(p, fallback_neighbourhood);
}

cv::Point View::Depth::project(const cv::Point3f &p) const noexcept {
    /* If there is no depth map and no projection, then projection is only
     * about removing the depth */
    if (projecter == nullptr) {
        return cv::Point(p.x, p.y);
    }

    return projecter->project(p);
}

std::vector<uint16_t> 
    View::Depth::default_neighbourhood = fallback_neighbourhood; 


View::View() noexcept 
    : depth(), c_bgr(nullptr), c_hsv(nullptr), c_yuv(nullptr), c_ycc(nullptr), 
      c_gray(nullptr), boundaries(), images(), ts(0) {}

View::~View() noexcept = default;

View::View(const View& other) noexcept
    : depth(other.depth), c_bgr(nullptr), c_hsv(nullptr), c_yuv(nullptr),
      c_ycc(nullptr), c_gray(nullptr), boundaries(other.boundaries),
      images(other.images), ts(other.ts) {
    reshortcut();
}

View::View(View&& other) noexcept
    : depth(std::move(other.depth)), c_bgr(nullptr), c_hsv(nullptr),
    c_yuv(nullptr), c_ycc(nullptr), c_gray(nullptr), 
    boundaries(std::move(other.boundaries)), images(std::move(other.images)), 
    ts(std::move(other.ts)) {
    reshortcut();
}

View& View::operator=(const View& other) noexcept {
    if (&other != this) {
        depth      = Depth(other.depth);
        c_bgr      = nullptr;
        c_hsv      = nullptr;
        c_yuv      = nullptr;
        c_ycc      = nullptr;
        c_gray     = nullptr;
        boundaries = other.boundaries;
        images     = other.images;
        ts         = other.ts;
        reshortcut();
    }

    return *this;
}

View& View::operator=(View&& other) noexcept {
    if (&other != this) {
        depth      = Depth(std::move(other.depth));
        c_bgr      = nullptr;
        c_hsv      = nullptr;
        c_yuv      = nullptr;
        c_ycc      = nullptr;
        c_gray     = nullptr;
        boundaries = std::move(other.boundaries);
        images     = std::move(other.images);
        ts         = std::move(other.ts);
        reshortcut();
    }

    return *this;
}

Error::Type View::use(cv::Mat data, Image::Mode mode) noexcept {
    ASSERT(mode.is_colour(),
           "View::Use::use(): Expecting a colour image but got a mode %d image "
           "instead!", static_cast<int>(mode));

    /* Update the timestamp if none is set */
    if (ts == 0) {
        auto now = std::chrono::high_resolution_clock::now();
        auto now_ms = 
            std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        ts = now_ms.time_since_epoch().count();
    }

    /* If there is already one such matrix, then we have a problem */
    auto it = images.find(mode);
    if (it != images.end()) {
        ASSERT((cv::sum(data != it->second.input()) == 
                cv::Scalar(0, 0, 0)),
                "View::use(): Changing the colour image of mode %d with a "
                "different one!", static_cast<int>(mode));
            return Error::INVALID_REQUEST;
    }
    
    if (cached_colour() != nullptr) {
        ASSERT( false,
                "View::use(): Changing the original colour image of mode %d "
                "with a new one of mode %d!", 
                static_cast<int>(cached_colour()->mode()),
                static_cast<int>(mode));
        return Error::INVALID_REQUEST;
    }

    auto p = images.emplace(std::piecewise_construct,
                            std::forward_as_tuple(mode), 
                            std::forward_as_tuple(std::move(data), 
                                                  std::move(mode)));
    
    auto &i = p.first->second;
    shortcut(i.mode(), &i);
    boundaries = i.frame();

    return Error::NONE;
}

Error::Type View::use(cv::Mat data, Image::Mode mode, 
                      const ProjectionDelegate &pd) noexcept {

    /* If this is a colour image then use the other use method meant for colour
     * images */
    if (mode.is_colour()) {
        return use(std::move(data), std::move(mode));
    }

    ASSERT(mode.is_depth(),
           "View::Use::use(): Expecting a depth image but got a mode %d image "
           "instead!", static_cast<int>(mode));

    /* If there is already one such matrix, then we have a problem */
    auto it = images.find(mode);
    if (it != images.end()) {
        ASSERT((cv::sum(data != it->second.input()) == cv::Scalar(0)),
                "View::use(): Changing the depth image of mode %d with a "
                "different one!", static_cast<int>(mode));
            return Error::INVALID_REQUEST;
    }

    if (cached_depth() != nullptr) {
        ASSERT( false,
                "View::use(): Changing the original depth map of mode %d "
                "with a new one of mode %d!", 
                static_cast<int>(cached_depth()->mode()),
                static_cast<int>(mode));
        return Error::INVALID_REQUEST;
    }

    auto p = images.emplace(std::piecewise_construct,
                            std::forward_as_tuple(mode), 
                            std::forward_as_tuple(std::move(data), 
                                                  std::move(mode)));

    /* Map this depth image in the depth object */
    auto &i = p.first->second;
    depth.map(i, pd);

    return Error::NONE;
}

const Image *View::cached(Image::Mode mode) const noexcept {
    /* If there is already one such image, then use it directly! */
    const auto it = images.find(mode);
    if (it != images.end()) {
        return &it->second;
    }
    
    return nullptr;
}

Image *View::cached(Image::Mode mode) noexcept {
    /* If there is already one such image, then use it directly! */
    auto it = images.find(mode);
    if (it != images.end()) {
        return &it->second;
    }
    
    return nullptr;
}

Image *View::cached_colour() noexcept {
    if (c_bgr != nullptr) {
        return c_bgr;
    }
    if (c_hsv != nullptr) {
        return c_hsv;
    }
    if (c_yuv != nullptr) {
        return c_yuv;
    }
    if (c_ycc != nullptr) {
        return c_ycc;
    }

    return nullptr;
}

Image *View::cached_depth() noexcept {
    Image *depth = nullptr;
    for (auto &p : images) {
            auto &i = p.second;
            if (i.mode().is_depth()) {
                depth = &i;
                if (i.mode() == Image::Mode::DEPTHF) {
                    return depth;
                }
            }
    }

    return depth;
}

Image View::image(const Image::Mode &mode, const cv::Rect &roi) noexcept {
    
    /* If there is already one such image, then use it directly! */
    auto im = cached(mode);
    if (im != nullptr) {
        return (*im)(roi);
    }

    /* Handle depth map differently fitt needs a scaler */
    if (mode.is_depth()) {
        auto d = cached_depth();
        if (d != nullptr) {
            return Image(*d, mode, roi, depth.scaler(d->mode(), mode));
        }
        ASSERT(false, "View::image(): Requesting a depth image but none is "
                      "available!"); 
    } else {

        /* Used the images bgr if available */
        if (c_bgr != nullptr) {
            return Image(*c_bgr, mode, roi);
        }

        /* Otherwise, generate the bgr sub image for generating the output */
        auto im = cached_colour();
        if (im != nullptr) {
            return Image(Image(*im, Image::Mode::BGR, roi), mode);
        }
        ASSERT(false, "View::image(): Requesting a colour image but none is "
                      "available!");
    }

    return Image::INVALID;
}

#define GENERATE_ROI_SHORTCUT(n,M) \
Image View::n(const cv::Rect &roi) noexcept { \
    if (c_##n != nullptr) {\
        return (*c_##n)(roi);\
    }\
\
    return image(Image::Mode::M, roi);\
}
GENERATE_ROI_SHORTCUT(bgr,  BGR)
GENERATE_ROI_SHORTCUT(hsv,  HSV)
GENERATE_ROI_SHORTCUT(yuv,  YUV)
GENERATE_ROI_SHORTCUT(ycc,  YCrCb)
GENERATE_ROI_SHORTCUT(gray, GRAY)

Image &View::image(const Image::Mode &mode) noexcept {   
    return cache(mode);
}

#define GENERATE_SHORTCUT(n,M) \
Image &View::n() noexcept { \
    if (c_##n != nullptr) {\
        return *c_##n;\
    }\
\
    return image(Image::Mode::M);\
}
GENERATE_SHORTCUT(bgr,  BGR)
GENERATE_SHORTCUT(hsv,  HSV)
GENERATE_SHORTCUT(yuv,  YUV)
GENERATE_SHORTCUT(ycc,  YCrCb)
GENERATE_SHORTCUT(gray, GRAY)

Image &View::cache(const Image::Mode &mode) noexcept {
     /* If there is already one such image, then use it directly! */
    auto im = cached(mode);
    if (im != nullptr) {
        return *im;
    }

    /* Handle depth map differently for it needs a scaler */
    if (mode.is_depth()) {
        auto d = cached_depth();
        if (d != nullptr) {
            auto scale = depth.scaler(d->mode(), mode);
            auto p = images.emplace(std::piecewise_construct,
                                    std::forward_as_tuple(mode), 
                                    std::forward_as_tuple(*im, mode,
                                                          scale));
            auto &d = p.first->second;
            depth.remap(d, true);
            return d;
        }
        ASSERT(false, "View::cache(): Requesting a depth image but none is "
                      "available!"); 
    } else {

        /* Use the BGR images (or generate it if not available) */
        if (c_bgr == nullptr) {
            auto im = cached_colour();
            if (im != nullptr) {
                auto m = Image::Mode(Image::Mode::BGR);
                auto p = images.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(m), 
                                        std::forward_as_tuple(*im, 
                                                              std::move(m)));
                c_bgr = &p.first->second;
            }
        }

        if (c_bgr == nullptr) {
            ASSERT(false, "View::image(): Requesting a colour image but "
                          "none is available!");
            return Image::INVALID;
        }

        /* Otherwise, cache the requested image generated from the bgr */
        auto p = images.emplace(std::piecewise_construct,
                                std::forward_as_tuple(mode), 
                                std::forward_as_tuple(*c_bgr, mode));
        auto &im = p.first->second;
        shortcut(im.mode(), &im);

        return im;
    }
            
    return Image::INVALID;
}

void View::shortcut(Image::Mode m, Image *i) noexcept {
    switch(m) {
        case Image::Mode::BGR:
            c_bgr = i;
            break;
        case Image::Mode::HSV:
            c_hsv = i;
            break;
        case Image::Mode::YUV:
            c_yuv = i;
            break;
        case Image::Mode::YCrCb:
            c_ycc = i;
            break;
        case Image::Mode::GRAY:
            c_gray = i;
            break;
        default:
            ASSERT(false, "View::shortcut(): Invalid shortcut requested "
                          "for image mode %d", static_cast<int>(m));
            break;
    }
}
    
void View::reshortcut() noexcept {
    for (auto &p : images) {
            auto &i = p.second;
            if (i.mode().is_colour() || i.mode().is_gray()) {
                shortcut(i.mode(), &i);
            }
    }

    auto d = cached_depth();
    if (d != nullptr) {
        depth.remap(*d);
    }
}

}  // namespace VPP
