/**
 *
 * @file      vpp/task/matcher.hpp
 *
 * @brief     This is a set of tasks aimed at running matching score evaluations
 *            for objects of different kinds
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

#include "vpp/task/matcher.hpp"

namespace VPP {
namespace Task {
namespace Matcher {
        
Matches Measures::extract(float threshold, bool exclusive_dst, 
                          bool exclusive_src) const noexcept {
    Matches matches;
    if(measurements.empty()) {
        return matches;
    }
    
    cv::Mat mask(cv::Mat::ones(measurements.rows, measurements.cols, CV_8U));

    while(true) {
        double    max_score = std::numeric_limits<double>::lowest();
        cv::Point max_location;
        cv::minMaxLoc(measurements, nullptr, &max_score, nullptr, &max_location,
                      mask);
        if (max_score < threshold) {
            return matches;
        }
       
        matches.emplace_back(max_location.y, max_location.x, max_score);
        mask.at<uint8_t>(max_location) = 0;
        if (exclusive_src) {
            mask.row(max_location.y) = 0;
        }
        if (exclusive_dst) {
            mask.col(max_location.x) = 0;
        }
    }
}

std::vector<float> Measures::scores(Match &m) const noexcept {
    std::vector <float> v;
    measurements.row(m.src).copyTo(v);
    return v;
}

std::vector<float> Measures::peers(Match &m) const noexcept {
    std::vector <float> v;
    measurements.col(m.dst).copyTo(v);
    return v;
}

}  // namespace Matcher
}  // namespace Task
}  // namespace VPP
