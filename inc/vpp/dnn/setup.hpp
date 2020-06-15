/**
 *
 * @file      vpp/dnn/setup.hpp
 *
 * @brief     This is the VPP DNN setup description file
 *
 * @details   This is the definition of the VPP DNN setup, a class that gathers
 *            any generic information for describing a DNN network.
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

#include "customisation/entity.hpp"
#include "customisation/file.hpp"

namespace VPP {
namespace DNN {

class Setup : public Parametrisable {
    public:
        Setup() noexcept;
        virtual ~Setup() noexcept; 

        virtual Customisation::Error setup() noexcept override;

        Customisation::File architecture;
        Customisation::File weights;
};

}  // namespace DNN
}  // namespace VPP
