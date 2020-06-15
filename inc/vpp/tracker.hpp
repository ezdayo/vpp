/**
 *
 * @file      vpp/tracker.hpp
 *
 * @brief     This is a 3D capable Kalman filter tracker implementation
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

#include <algorithm>
#include <opencv2/core/core.hpp>
#include <opencv2/video/tracking.hpp>

#include "vpp/log.hpp"
#include "vpp/util/ocv/coordinates.hpp"

namespace VPP {
namespace Tracker {

/** Class Measure 
 *  A measure is composed of a 3d centre point c=(cx, cy, cz) expressed in
 *  meters, a a width and an height. It can be accessed either as a 5-float
 *  raw array, by name or as a matrix */
class Measure {
    public:
        static const int length = 5;

        inline Measure() noexcept : centre(), size(), 
                                    measure(length, 1, CV_32F, &centre.x, 
                                            cv::Mat::AUTO_STEP) {}

        inline Measure(const Measure &other) noexcept 
            : centre(other.centre), size(other.size),
              measure(length, 1, CV_32F, &centre.x, cv::Mat::AUTO_STEP) {}
       
        /* Internal values represented either as an array or a structure */
        Util::OCV::Triplet centre;
        Util::OCV::Couple  size;

        inline operator cv::Mat&() noexcept {
            return measure;
        }

        inline operator const cv::Mat&() const noexcept {
            return measure;
        }

        inline float &operator[] (int id) noexcept {
            ASSERT(id < length && id >=0,
                   "Invalid measure index provided %d!", id);
            return *(&centre.x+id);
        }

    private:
        cv::Mat measure;
};

/** Class State 
 *  A state is composed of a 3d centre point c=(c.x, c.y, c.z) expressed in
 *  meters a speed vector v=(v.x, v.y, v.z) a width and an height. It can be
 *  accessed either as a 8-float raw array, by name or as a matrix and a state
 *  can be predicted and tracked as a Kalman filter */
class State : public cv::KalmanFilter {
    public:
        static const int length = 8;
        static State DEFAULT;

        State() noexcept;
        State(const State &other) noexcept;

        /* Internal values represented either as an array or a structure */
        Util::OCV::Triplet centre;
        Util::OCV::Triplet speed;
        Util::OCV::Couple  size;

        State &operator = (const Measure &measure) noexcept;

        bool valid() const noexcept {
            return validity > 0;
        }

        float accuracy() const noexcept {
            return std::max(validity, 0.0f)/timeout;
        }

        void predictability(float time) noexcept;

        inline operator cv::Mat&() noexcept {
            return state;
        }

        inline operator const cv::Mat&() const noexcept {
            return state;
        }

        inline operator Measure() const noexcept {
            Measure m;
            m.centre = centre;
            m.size   = size;
            return m;
        }

        inline float &operator[] (int id) noexcept {
            ASSERT(id < length && id >=0,
                   "Invalid state index provided %d!", id);
            return *(&centre.x+id);
        }

        float similarity(const State &other) const noexcept;
        void predict(float dt) noexcept;
        void correct(const Measure &measure) noexcept;

    private:
        cv::Mat state;
        float   validity;
        float   timeout;
};

} // namespace Tracker
} // namespace VPP
