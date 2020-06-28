/**
 *
 * @file      vpp/scene.hpp
 *
 * @brief     This is the VPP scene description file
 *
 * @details   This file describes the structure of a zone within an image and
 *            adds a few methods to handle it. A zone is basically a bounding
 *            box around a point of interest provided with a context, i.e. a
 *            most likely prediction. It also comes with a list of other and 
 *            refined predictions, a short description, a refined contour if
 *            required, as well as a state for storing its actual coordinates
 *            and estimated speed (if tracking is enabled).
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
#include <opencv2/imgproc.hpp>
#include <vector>

#include "vpp/log.hpp"
#include "vpp/prediction.hpp"
#include "vpp/coordinates.hpp"

namespace VPP {

/** Bounding box type */
class BBox : public cv::Rect {

    public:
        /* Uncropped bounding boxes */
        BBox() noexcept : cv::Rect({}) {}

        BBox(cv::Rect bbox) noexcept : cv::Rect(std::move(bbox)) {}

        BBox(int left, int top, int right, int bottom) noexcept
            : cv::Rect(left, top, right-left, bottom-top) {}
    
        /* Floating-point bounding boxes */
        BBox(cv::Rect_<float> box, const cv::Rect &frame) noexcept
            : BBox(std::move(cv::Rect(box.x*frame.width, box.y*frame.height, 
                                      box.width*frame.width,
                                      box.height*frame.height))) {} 

        BBox(cv::Rect_<float> box, int width, int height) noexcept
            : BBox(std::move(box), cv::Rect(0, 0, width, height)) {} 

        BBox(cv::Rect_<float> box, const cv::Mat &image) noexcept
            : BBox(std::move(box), image.cols, image.rows) {} 
    
        BBox(float left, float top, float right, float bottom,
             int width, int height) noexcept
            : BBox(cv::Rect_<float>(left, top, right-left, bottom-top), 
                   width, height) {}

        BBox(float left, float top, float right, float bottom,
             const cv::Mat &image) noexcept
            : BBox(left, top, right, bottom, image.cols, image.rows) {}
};

/** Zone of interest type */
class Zone;
using Zones = std::vector<std::reference_wrapper<Zone>>;
using ConstZones = std::vector<std::reference_wrapper<const Zone>>;
using Contour = std::vector<cv::Point>;
using Contours = std::vector<std::reference_wrapper<Contour>>;
using ConstContours = std::vector<std::reference_wrapper<const Contour>>;

class Zone final : public BBox {

    public:
        /** 
         *  Class Measure
         *
         *  A Zone measure is composed of a 3d centre point c=(cx, cy, cz)
         *  expressed in meters, a width and an height. It can be accessed
         *  either as a 5-float raw array, by name or as a matrix 
         */
       class Measure {
            public:
                static const int length = 5;

                inline Measure() noexcept 
                    : centre(), size(), measure(length, 1, CV_32F, &centre.x, 
                                                cv::Mat::AUTO_STEP) {}

                inline Measure(const Measure &other) noexcept 
                    : centre(other.centre), size(other.size),
                      measure(length, 1, CV_32F, &centre.x, 
                              cv::Mat::AUTO_STEP) {}

                inline Measure(Measure&& other) noexcept 
                    : centre(std::move(other.centre)),
                      size(std::move(other.size)),
                      measure(length, 1, CV_32F, &centre.x, 
                              cv::Mat::AUTO_STEP) {}

                inline Measure& operator=(const Measure& other) noexcept {
                    centre = other.centre;
                    size   = other.size;
                    measure = cv::Mat(length, 1, CV_32F, &centre.x, 
                                      cv::Mat::AUTO_STEP);
                    return *this;
                }

                inline Measure& operator=(Measure&& other) noexcept {
                    centre = std::move(other.centre);
                    size   = std::move(other.size);
                    measure = cv::Mat(length, 1, CV_32F, &centre.x, 
                                      cv::Mat::AUTO_STEP);
                    return *this;
                }
                
                inline ~Measure() = default;

                Triplet centre;
                Couple  size;

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

        /** 
         *  Class State
         *
         *  A Zone state is composed of a 3d centre point c=(c.x, c.y, c.z)
         *  expressed in meters, a width and an height in the same units and a 
         *  speed vector v=(v.x, v.y, v.z) expressed in m/s. It can be accessed
         *  either as a 8-float raw array, by name or as a matrix.
         */
        class State {
            public:
                static const int length = 8;

                /* Constructor and copy constructor */
                inline State() noexcept 
                    : centre(), size(), speed(),
                      state(length, 1, CV_32F, &centre.x, cv::Mat::AUTO_STEP) {}

                inline State(const State &other) noexcept
                    : centre(other.centre), size(other.size),
                      speed(other.speed), 
                      state(length, 1, CV_32F, &centre.x, cv::Mat::AUTO_STEP) {}

                inline State(State &&other) noexcept
                    : centre(std::move(other.centre)), 
                      size(std::move(other.size)),
                      speed(std::move(other.speed)), 
                      state(length, 1, CV_32F, &centre.x, cv::Mat::AUTO_STEP) {}

                inline State &operator = (const State &other) noexcept {
                    centre = other.centre;
                    size   = other.size;
                    speed  = other.speed;
                    return *this;
                }

                inline State &operator = (State &&other) noexcept {
                    centre = std::move(other.centre);
                    size   = std::move(other.size);
                    speed  = std::move(other.speed);
                    return *this;
                }

                inline State &operator = (const Measure &measure) noexcept {
                    centre = measure.centre;
                    size   = measure.size;
                    speed  = Triplet();
                    return *this;
                }

                inline ~State() = default;

                /* Geometry and displacement */
                Triplet  centre;
                Couple   size;
                Triplet  speed;

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

            private:
                cv::Mat state;
        };

        /* Universal unique identifier for a zone */
        uint64_t uuid;

        State                   state;
        Contour                 contour;
        std::vector<Prediction> predictions;
        Prediction              context;
        std::string             description;
        int                     marked;

        Zone() noexcept : BBox(), state(), contour(), predictions(), context(),
                          description(), marked(0) {}

        ~Zone() noexcept = default;

        /* Zones can be copied and moved unconditionally */
        Zone(const Zone& other) = default;
        Zone(Zone&& other) = default;
        Zone& operator=(const Zone& other) = default;
        Zone& operator=(Zone&& other) = default;

        Zone(BBox bbox) noexcept : BBox(std::move(bbox)), state(), contour(), 
                                   predictions(), context(), description(),
                                   marked(0) {}
   
        Zone(BBox bbox, Prediction pred) noexcept
            : BBox(std::move(bbox)), state(), contour(), predictions({ pred }),
              context(std::move(pred)), description(), marked(0) {}

        Zone(BBox bbox, std::vector<Prediction> preds) noexcept;

        Zone(BBox bbox, Contour c) noexcept 
                : BBox(std::move(bbox)), state(), contour(std::move(c)),
                  predictions(), context(), description(), marked(0) {}

        Zone(Contour c) noexcept
                : BBox(std::move(cv::boundingRect(c))), state(),
                  contour(std::move(c)), predictions(), context(),
                  description(), marked(0) {}

        Zone &predict(Prediction pred) noexcept;
        Zone &predict(std::vector<Prediction> preds) noexcept;

        Zone &update(Zone &older) noexcept;

        Zone &merge(const Zone &zone) noexcept;
        static Zone merge(const Zones &zones) noexcept;
       
        inline void invalidate() noexcept {
            marked = 0;
        }

        inline bool valid() const noexcept {
            return marked > 0;
        }

        inline bool invalid() const noexcept {
            return marked <= 0;
        }

        static bool when_valid(const Zone &zone) noexcept {
            return zone.valid();
        } 

        static bool when_invalid(const Zone &zone) noexcept {
            return zone.invalid();
        } 

        inline Zone &describe(std::string desc) noexcept {
            description = std::move(desc);
            return *this;
        }

        inline bool inside(Zone &other) const noexcept {
            return ((*this & other).area() > (area()*0.95));
        }
};

class ZoneFilterDelegate {
    public:
        virtual bool filter(const Zone &zone) const noexcept = 0;            
};

}  // namespace VPP
