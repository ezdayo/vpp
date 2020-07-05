/**
 *
 * @file      vpp/prediction.hpp
 *
 * @brief     This is the VPP prediction description file
 *
 * @details   This file describes the structure of a prediction and adds a few
 *            methods to handle it. A prediction is basically a score, a list 
 *            index and an object index within this list.
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

#include <cstdint>
#include <set>

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

        inline bool is_in(const std::set<int32_t> &valid) const noexcept {
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

}  // namespace VPP
