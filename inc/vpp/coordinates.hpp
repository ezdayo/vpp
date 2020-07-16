/**
 *
 * @file      vpp/coordinates.hpp
 *
 * @brief     This is a 3D and 2D coordinates utility function class
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

#include <opencv2/core/core.hpp>

#include "vpp/log.hpp"

namespace VPP {

/* Two-uplet class */
class Couple {
    public:
        static const int length = 2;

        inline Couple() noexcept = default;
        inline ~Couple() noexcept = default;
        inline Couple(const Couple& other) = default;
        inline Couple(Couple&& other) = default;
        inline Couple& operator=(const Couple& other) = default;
        inline Couple& operator=(Couple&& other) = default;

        float x;
        float y;
         
        inline Couple& operator = (const cv::Point2i &p) {
            x = p.x;
            y = p.y;

            return *this;
        }

        inline Couple& operator = (const cv::Point2f &p) {
            x = p.x;
            y = p.y;

            return *this;
        }

        inline operator cv::Mat() noexcept {
            return cv::Mat(length, 1, CV_32F, static_cast<void *>(&x),
                           cv::Mat::AUTO_STEP);
        }

        inline operator const cv::Mat() const noexcept {
            return cv::Mat(length, 1, CV_32F, 
                           const_cast<void *>(static_cast<const void *>(&x)),
                           cv::Mat::AUTO_STEP);
        }

        inline operator cv::Rect2f() const noexcept {
            return cv::Rect2f(0, 0, x, y);
        }

        inline operator cv::Size2f() const noexcept {
            return cv::Size2f(x, y);
        }

        inline operator cv::Point2f() const noexcept {
            return cv::Point2f(x, y);
        }

        inline float &operator[] (int id) noexcept {
            ASSERT(id < length && id >=0,
                   "Invalid pair index provided %d!", id);
            return *(&x+id);
        }

        inline float iou(const Couple &other) const noexcept {
            cv::Rect2f a(*this);
            cv::Rect2f b(other);
            auto top = static_cast<float>((a & b).area());
            if (top == 0) {
                return 0;
            }
            auto bot = static_cast<float>((a | b).area());

            return top / bot;
        }

        inline float square() const noexcept {
            auto d = cv::Point2f(x, y);
            return (d.x * d.x) + (d.y * d.y);
        }

        inline float square_dist(const Couple &other) const noexcept {
            auto d = cv::Point2f(x, y) - cv::Point2f(other.x, other.y);
            return (d.x * d.x) + (d.y * d.y);
        }
};

/* Three-uplet class */
class Triplet {
    public:
        static const int length = 3;

        inline Triplet() noexcept = default;
        inline ~Triplet() noexcept = default;
        inline Triplet(const Triplet& other) = default;
        inline Triplet(Triplet&& other) = default;
        inline Triplet& operator=(const Triplet& other) = default;
        inline Triplet& operator=(Triplet&& other) = default;

        float x;
        float y;
        float z;

        inline Triplet& operator = (const cv::Point3f &p) {
            x = p.x;
            y = p.y;
            z = p.z;

            return *this;
        }

        inline Triplet& operator = (const cv::Point3i &p) {
            x = p.x;
            y = p.y;
            z = p.z;

            return *this;
        }

        inline operator cv::Mat() noexcept {
            return cv::Mat(length, 1, CV_32F, static_cast<void *>(&x),
                           cv::Mat::AUTO_STEP);
        }

        inline operator const cv::Mat() const noexcept {
            return cv::Mat(length, 1, CV_32F, 
                           const_cast<void *>(static_cast<const void *>(&x)),
                           cv::Mat::AUTO_STEP);
        }

        inline operator cv::Point3f() const noexcept {
            return cv::Point3f(x, y, z);
        }

        inline float &operator[] (int id) noexcept {
            ASSERT(id < length && id >=0,
                   "Invalid triplet index provided %d!", id);
            return *(&x+id);
        }

        inline float square() const noexcept {
            auto d = cv::Point3f(x, y, z);
            return (d.x * d.x) + (d.y * d.y) + (d.z * d.z);
        }

        inline float square_dist(const Triplet &other) const noexcept {
            auto d = cv::Point3f(x, y, z) - 
                     cv::Point3f(other.x, other.y, other.z);
            return (d.x * d.x) + (d.y * d.y) + (d.z * d.z);
        }
};

}  // namespace VPP
