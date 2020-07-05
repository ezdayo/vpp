/**
 *
 * @file      vpp/view.hpp
 *
 * @brief     This is the VPP view description file
 *
 * @details   This file describes the structure of all aspects related to the
 *            capture of the visual environment, be it the image in various
 *            formats, the depths of the scene (if it exists), as well as
 *            helper methods to access the information in an efficient way. As
 *            such, caching is extensively used to only make computationally
 *            expensive operations once.
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

#include <unordered_map>

#include "vpp/error.hpp"
#include "vpp/image.hpp"
#include "vpp/projection.hpp"

namespace VPP {

class View final {
    public: 
        class Depth final {
            public:
                Depth() noexcept : neighbourhood(), depth_map(nullptr), 
                                   projecter(nullptr) {};

                ~Depth() noexcept = default;

                Depth(const Depth& other) noexcept 
                    : neighbourhood(other.neighbourhood), depth_map(nullptr), 
                      projecter(other.projecter) {}

                Depth(Depth&& other) noexcept
                    : neighbourhood(std::move(other.neighbourhood)), 
                      depth_map(nullptr), projecter(other.projecter) {}

                Depth& operator=(const Depth& other) noexcept {
                    neighbourhood = other.neighbourhood;
                    depth_map     = nullptr; 
                    projecter     = other.projecter;
                    return *this;
                }

                Depth& operator=(Depth&& other) noexcept {
                    neighbourhood = std::move(other.neighbourhood);
                    depth_map     = nullptr; 
                    projecter     = other.projecter;
                    return *this;
                }

                Error::Type map(const Image &d, const Projecter &p) noexcept;
                Error::Type remap(const Image &d, bool force=false) noexcept;

                /* Returns -1 if the depth is not valid */
                float at(const cv::Point &pix) const noexcept;

                /* Returns the average of the valid depth points */
                float at(const cv::Rect &area) const noexcept;

                /* Scaling factor from first to second mode */
                float scaler(const Image::Mode &from,
                             const Image::Mode &to) const noexcept;

                /* Projections back and forth */
                cv::Point3f deproject(const cv::Point &p, 
                                      float z) const noexcept;
                cv::Point3f deproject(const cv::Point &p,
                                      const std::vector<uint16_t> &neighbours) 
                    const noexcept;
                cv::Point3f deproject(const cv::Point &p) const noexcept;
                cv::Point project(const cv::Point3f &p) const noexcept;

                /* Neighbourhood for finding the right depth because of holes
                 * in the depth map*/
                std::vector<uint16_t>        neighbourhood;
                static std::vector<uint16_t> default_neighbourhood;

            private:
                Image const     *depth_map;
                Projecter const *projecter;
        };

        View() noexcept;
        ~View() noexcept;

        /* A view can be copied, in which case, only the original input images
         * will be kept using the image copy constructors and assignments.
         * When a view is moved, its internal shortcuts shall be reconfigured
         * and this is exactly the purpose of the specific move constructors
         * and assigmnents */
        View(const View& other) noexcept;
        View(View&& other) noexcept;
        View& operator=(const View& other) noexcept;
        View& operator=(View&& other) noexcept;

        inline bool empty() const noexcept {
            return images.empty();
        }

        inline const cv::Rect & frame() const noexcept {
            return boundaries;
        }

        /* Adding a new image in the view. Beware that only one colour image
         * and one depth map can be used in a single view.*/
        Error::Type use(cv::Mat data, Image::Mode mode) noexcept;
        Error::Type use(cv::Mat depth, Image::Mode mode, 
                        const ProjectionDelegate &pd) noexcept;

        /* Check if there is any such kind of images cached */
        const Image *cached(Image::Mode mode) const noexcept;
        Image *cached(Image::Mode mode) noexcept;
        Image *cached_colour() noexcept;
        Image *cached_depth() noexcept;

        /* "Uncached" image, i.e. should conversion happens, those are not
         * cached back, but if a cached version is found, then use it!
         * Besides, if done on a small region of interest, the conversion
         * is only done on this region of interest */
        Image image(const Image::Mode &mode, const cv::Rect &roi) noexcept;
        Image bgr(const cv::Rect &roi) noexcept;
        Image hsv(const cv::Rect &roi) noexcept;
        Image yuv(const cv::Rect &roi) noexcept;
        Image ycc(const cv::Rect &roi) noexcept;
        Image gray(const cv::Rect &roi) noexcept;

        /* "Cached" image, i.e. if some conversion is required, then cache the
         * conversion done and return a reference to it */
        Image &image(const Image::Mode &mode) noexcept;
        Image &bgr() noexcept;
        Image &hsv() noexcept;
        Image &yuv() noexcept;
        Image &ycc() noexcept;
        Image &gray() noexcept;

        /* Force caching a certain mode of the image */
        Image &cache(const Image::Mode &mode) noexcept;

        /* Proxy to get depth information from a view (if any) */
        Depth depth;

    private:
        void shortcut(Image::Mode m, Image *i) noexcept;
        void reshortcut() noexcept;
        Image *                        c_bgr;
        Image *                        c_hsv;
        Image *                        c_yuv;
        Image *                        c_ycc;
        Image *                        c_gray;
        cv::Rect                       boundaries;
        std::unordered_map<int, Image> images;
};

}  // namespace VPP
