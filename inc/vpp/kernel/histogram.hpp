/**
 *
 * @file      vpp/kernel/histogram.hpp
 *
 * @brief     This is the VPP histogram kernel description file
 *
 * @details   This kernel is an histogram computation and matching kernel
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

#include <map>
#include <opencv2/imgproc.hpp>
#include <utility>

#include "customisation.hpp"
#include "vpp/scene.hpp"
#include "vpp/zone.hpp"

namespace VPP {
namespace Kernel {

class Histogram : public Parametrisable {
    public:
        class Parameters {
            public:
                class Histogram {
                    public:        
                        int                         mode;
                        int                         entries;
                        std::vector<const float *>  ranges;
                        std::vector<int>            sizes;
                        std::vector<int>            channels;
                        /* Storage space for the ranges */
                        std::vector<float>          storage;
                };

                class Mask {
                    public:
                        bool         valid;   
                        cv::Scalar   low;
                        cv::Scalar   high;
                };

                bool operator == (const Parameters &other) const noexcept;

                Mask      mask;
                Histogram histogram;
        };

        class Data {
            public:
                explicit Data(Parameters &params, View &view,
                              const cv::Rect &zone) noexcept;
                ~Data() noexcept;

                /* Preparing the input frame [DEPRECATED use cache!]*/ 
                static void prepare_input(View &view,
                                          const Image::Mode &mode) noexcept;
       
                /* Updating the histogram (recalculating it) */ 
                void update(View &view, const cv::Rect &zone) noexcept;

                /* Comparing with another histogram */
                double compare(Data &other, 
                               enum cv::HistCompMethods method) const noexcept;

                /* Back projecting the histogram */
                cv::Mat back_project(View &view) const noexcept;

                cv::Mat     signature;
                cv::Mat     mask;

                Parameters &config;
        };

        class Ranges : public Parametrisable {
            public:
                Ranges() noexcept;
                ~Ranges() noexcept;
 
                Customisation::Error setup() noexcept override;

                PARAMETER(Direct, Saturating, Immediate,
                          std::vector<float>)               low;
                PARAMETER(Direct, Saturating, Immediate,
                          std::vector<float>)               high;
        };

        explicit Histogram() noexcept;
        ~Histogram() noexcept;

        Customisation::Error setup() noexcept override;

        /* The list of channels to be used in the histogram */
        PARAMETER(Mapped, Bounded, Immediate, std::vector<int>)        channels;

        /* The configuration for each channel */
        Ranges                                                         mask;
        Ranges                                                         ranges;
        PARAMETER(Direct, Saturating, Immediate, std::vector<int32_t>) bins;

        Data &data(View &view, const Zone &zone) noexcept;

        Image::Mode mode() const noexcept {
            return config.histogram.mode;
        }

    protected:
        std::map<uint64_t, Data> tracked;
        Parameters               config;
};

}  // namespace Kernel
}  // namespace VPP
