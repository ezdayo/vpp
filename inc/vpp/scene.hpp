/**
 *
 * @file      vpp/scene.hpp
 *
 * @brief     This is the VPP scene description file
 *
 * @details   This file describes the structure of the token that flows in the
 *            visual pipeline processor (VPP). This token is called a scene and
 *            consists of a timestamp, an image and a list of zones of interest.
 *            Each zone is a bounding box in the image with a context, a short
 *            description, an emergency flag, and a list of predictions.
 *            Each prediction is a score, a list index and an object index.
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
#include <cstdint>
#include <functional>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <set>
#include <utility>
#include <vector>

#include "vpp/log.hpp"
#include "vpp/tracker.hpp"
#include "vpp/util/ocv/projection.hpp"

namespace VPP {

class Prediction final {
    public:
        Prediction() noexcept : score(-1), dataset(-1), id(-1) {}

        Prediction(float score_, int16_t dataset_, int16_t id_) noexcept
            : score(std::move(score_)), dataset(std::move(dataset_)), 
              id(std::move(id_)) {}

        Prediction(int score_, int16_t dataset_, int16_t id_) noexcept
            : score(static_cast<float>(score_)/100.0),
              dataset(std::move(dataset_)), id(std::move(id_)) {}

        static inline int32_t gid(int16_t dataset_, int16_t id_) {
            return static_cast<int32_t>(dataset_)*65536 + id_;
        }

        inline int32_t gid() const noexcept {
            return gid(dataset, id);
        }

        inline bool is_a(int16_t dataset_, int16_t id_) const noexcept {
            return ((id_ == id) && (dataset_ == dataset));
        }

        inline bool is_in(std::set<int32_t> valid) const noexcept {
            return (valid.find(gid()) != valid.end());
        }

        inline bool operator == (const Prediction &other) const noexcept {
            return (score == other.score);
        }

        inline bool operator != (const Prediction &other) const noexcept {
            return (score != other.score);
        }

        inline bool operator < (const Prediction &other) const noexcept {
            return (score < other.score);
        }

        inline bool operator > (const Prediction &other) const noexcept {
            return (score > other.score);
        }

        inline bool operator <= (const Prediction &other) const noexcept {
            return (score <= other.score);
        }

        inline bool operator >= (const Prediction &other) const noexcept {
            return (score >= other.score);
        }

        /**# Prediction score */
        float score;

        /**# Index of the dataset for the prediction */
        int16_t dataset;

        /**# Index in the dataset */
        int16_t id;
};

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
        Tracker::State          tracked;
        Contour                 contour;
        std::vector<Prediction> predictions;
        Prediction              context;
        std::string             description;
        int                     step;

        Zone() noexcept : BBox(), tracked(Tracker::State::DEFAULT), contour(),
                          predictions(), context(), description(), step(0) {}

        Zone(BBox bbox) noexcept : BBox(std::move(bbox)),
                                   tracked(Tracker::State::DEFAULT),
                                   contour(), predictions(), context(), 
                                   description(), step(0) {}
   
        Zone(BBox bbox, Prediction pred) noexcept
            : BBox(std::move(bbox)), tracked(Tracker::State::DEFAULT),
              contour(), predictions({ pred }), context(std::move(pred)),
              description(), step(0) {}

        Zone(BBox bbox, std::vector<Prediction> preds) noexcept;

        Zone(BBox bbox, Contour c) noexcept 
                : BBox(std::move(bbox)), tracked(Tracker::State::DEFAULT),
                  contour(std::move(c)), predictions(), context(), 
                  description(), step(0) {}

        Zone(Contour c) noexcept
                : BBox(std::move(cv::boundingRect(c))),
                  tracked(Tracker::State::DEFAULT), contour(std::move(c)), 
                  predictions(), context(), description(), step(0) {}

        Zone(const Zone &other) noexcept 
                : BBox(static_cast<cv::Rect>(other)),
                  tracked(other.tracked), contour(other.contour),
                  predictions(other.predictions), context(other.context),
                  description(other.description), step(other.step) {}

        Zone &predict(Prediction pred) noexcept;
        Zone &predict(std::vector<Prediction> preds) noexcept;
        
        bool predict(float dt) noexcept;
        Zone &update(Zone &older) noexcept;

        Zone &merge(const Zone &zone) noexcept;
        static Zone merge(const Zones &zones) noexcept;
        
        inline Zone &describe(std::string desc) noexcept {
            description = std::move(desc);
            return *this;
        }

        inline bool inside(Zone &other) const noexcept {
            return ((*this & other).area() > (area()*0.95));
        }

        /* Evaluating how two zones can be similar (or not) */
        float similarity(const Zone &other) const noexcept;
};

class ZoneFilterDelegate {
    public:
        virtual bool filter(const Zone &zone) const noexcept = 0;            
};

/** Scene type */
class Scene final {
    public:
        using ZoneFilter = 
            std::function<bool (const Zone &) noexcept>;

        Scene() noexcept;

        /* using either a depth or color input matrix */
        void use(cv::Mat color) noexcept;
        void use(cv::Mat depth,
                 const Util::OCV::ProjectionDelegate &pd) noexcept;

        uint16_t depth_at(const cv::Point &pix) const noexcept;
        cv::Point3f point_at(const cv::Point &pix, uint16_t z) const noexcept;
        inline cv::Point3f point_at(const cv::Point &pix) const noexcept {
            return point_at(pix, depth_at(pix));
        }
        cv::Point pixel_at(const cv::Point3f &p) const noexcept;

        inline uint64_t timestamp() const noexcept {
            return ts;
        }

        inline const cv::Mat & input() const noexcept {
            return iimage;
        }

        const cv::Mat & output() const noexcept;
        cv::Mat & drawable() noexcept;

        inline const cv::Mat & depth() const noexcept {
            return dmap;
        }

        /* flushing is copying the original scene on the drawable surface */
        void flush() noexcept;

        inline const cv::Rect & frame() noexcept {
            return fimage;
        }

        inline bool broken() const noexcept {
            return iimage.empty();
        }

        inline bool empty() const noexcept {
            return areas.empty();
        }

        Zone &mark(Zone zone) noexcept;
        
        inline Zone &mark(BBox bbox) noexcept {
            /* Relying on copy-elision for performance */
            return mark(Zone(bbox));
        }

        inline Zone &mark(cv::Rect bbox) noexcept {
            /* Relying on copy-elision for performance */
            return mark(Zone(BBox(bbox)));
        }

        inline Zone &mark(cv::Rect_<float> bbox) noexcept {
            /* Relying on copy-elision for performance */
            return mark(Zone(BBox(bbox, fimage)));
        }

        ConstZones zones() const noexcept;
        Zones zones() noexcept;

        Zones zones(const ZoneFilterDelegate &f) noexcept;
        ConstZones zones(const ZoneFilterDelegate &f) const noexcept;
        Zones zones(const ZoneFilter &f) noexcept;
        ConstZones zones(const ZoneFilter &f) const noexcept;

        std::vector<Zone> extract(const ZoneFilterDelegate &f) noexcept;
        std::vector<Zone> extract(const ZoneFilter &f) noexcept;

        /* Remembering a scene for tracking: everything is copied except the
         * images (useless) */
        Scene remember() const noexcept;

        /* Predicting an old scene from a newer one (using timestamp) */
        void predict(const Scene &newer) noexcept;

        /* Updating a scene from an old one (using zone overlapping) and
         * returns a copy of the scene */
        Scene update(Scene &older) noexcept;

    private:
        std::vector<Zone>                    areas;
        uint64_t                             ts;
        cv::Rect                             fimage;
        cv::Mat                              iimage;
        cv::Mat                              oimage;
        cv::Rect                             fdmap;
        cv::Mat                              dmap;
        Util::OCV::ProjectionDelegate const *projection;
};

}  // namespace VPP
