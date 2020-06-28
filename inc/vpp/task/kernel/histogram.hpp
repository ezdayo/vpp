/**
 *
 * @file      vpp/task/kernel/histogram.hpp
 *
 * @brief     These are various tasks to efficiently use Histograms kernels in 
 *            a scene
 *
 * @details   This is a collection of tasks for helping in the efficient
 *            implementation of a Histogram-based algorithms in a scene
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

#include "customisation/parameter.hpp"
#include "vpp/kernel/histogram.hpp"
#include "vpp/error.hpp"
#include "vpp/task.hpp"

namespace VPP {
namespace Task {
namespace Kernel {
namespace Histogram {

class CamShift : public Tasks::ForZones {
    public:
        explicit CamShift(const int mode, VPP::Kernel::Histogram &h) noexcept;
        virtual ~CamShift() noexcept = default;

        /* To be called with the "historic" zones on the new scene */
        Error::Type start(Scene &s, Zones &zs) noexcept;

     protected:
        virtual Error::Type process(Scene &s, Zone &z)
            noexcept override final;

    private:
        VPP::Kernel::Histogram &histogram;
        cv::TermCriteria        term;
};

}  // namespace Histogram
}  // namespace Kernel
}  // namespace Task
}  // namespace VPP
