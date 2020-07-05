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

#include <opencv2/imgproc.hpp>
#include <utility>

#include "customisation.hpp"
#include "vpp/kernel.hpp"
#include "vpp/scene.hpp"
#include "vpp/zone.hpp"

namespace VPP {
namespace Kernel {
namespace Histogram {
        
struct Parameters {
    struct Mask {
        bool                        valid;   
        cv::Scalar                  low;
        cv::Scalar                  high;
    };

        int                         mode;
        int                         entries;
        std::vector<const float *>  ranges;
        std::vector<int>            sizes;
        std::vector<int>            channels;
        /* Storage space for the ranges */
        std::vector<float>          storage;

        Mask                        mask;
        
        bool operator == (const Parameters &other) const noexcept;
};

class Context : public VPP::Kernel::Context {
    public:
        explicit Context(Zone &zone, const Zone::Copier &copier,
                         unsigned int sz, Parameters &params) noexcept;
        ~Context() noexcept = default;

        /* Initialising the histogram (recalculating it) */ 
        void initialise(View &view) noexcept;

        /* Comparing with another histogram */
        double compare(Context &other, 
                       enum cv::HistCompMethods method) const noexcept;

        /* Back projecting the histogram */
        cv::Mat back_project(View &view) const noexcept;

        cv::Mat     signature;
        cv::Mat     mask;

    protected:
        Parameters &config;
};

using Contexts = std::vector<std::reference_wrapper<Context>>;

class Engine : public VPP::Kernel::Engine<Engine, Context> {
    public:
        class Ranges : public Parametrisable {
            public:
                Ranges() noexcept;
                ~Ranges() noexcept = default;
 
                Customisation::Error setup() noexcept override;

                PARAMETER(Direct, Saturating, Immediate,
                          std::vector<float>)               low;
                PARAMETER(Direct, Saturating, Immediate,
                          std::vector<float>)               high;
        };

        using Parent = VPP::Kernel::Engine<Engine, Context>;

        explicit Engine(const Zone::Copier &c, unsigned int sz) noexcept;
        ~Engine() noexcept = default;

        Customisation::Error setup() noexcept override;

        Customisation::Error clear() noexcept override;

        /* The list of channels to be used in the histogram */
        PARAMETER(Mapped, Bounded, Immediate, std::vector<int>)        channels;

        /* The configuration for each channel */
        Ranges                                                         mask;
        Ranges                                                         ranges;
        PARAMETER(Direct, Saturating, Immediate, std::vector<int32_t>) bins;

        void prepare(const Zones &zs) noexcept;

        Image::Mode mode() const noexcept {
            return config.mode;
        }

    protected:
        Parameters               config;
};

}  // namespace Histogram
}  // namespace Kernel
}  // namespace VPP
