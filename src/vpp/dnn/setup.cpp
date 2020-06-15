/**
 *
 * @file      vpp/dnn/setup.cpp
 *
 * @brief     This is the VPP DNN setup implementation file
 *
 * @details   This is the implementation of a generic VPP DNN setup.
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

#include "vpp/log.hpp"
#include "vpp/dnn/setup.hpp"

namespace VPP {
namespace DNN {

Setup::Setup() noexcept
    : Customisation::Entity("Setup"), architecture(""), weights("") {
        architecture.denominate("architecture")
                    .describe("The network architecture configuration file")
                    .characterise(Customisation::Trait::CONFIGURABLE);
        expose(architecture);

        weights.denominate("weights")
               .describe("The network weights configuration file")
               .characterise(Customisation::Trait::CONFIGURABLE);
        expose(weights);
}

Setup::~Setup() noexcept = default;

Customisation::Error Setup::setup() noexcept {

    // Archietcture can be left undefined for OCV, so let's accept this case
    if ( (!architecture.undefined()) && (!architecture.exists()) ) {
        LOGE("%s[%s]::setup(): Cannot find network architecture file '%s'!", 
             value_to_string().c_str(), name().c_str(), 
             static_cast<std::string>(architecture).c_str());
        return Customisation::Error::NOT_EXISTING;
    }

    if (!weights.exists()) {
        LOGE("%s[%s]::setup(): Cannot find network weights file '%s'!", 
             value_to_string().c_str(), name().c_str(),
             static_cast<std::string>(weights).c_str());
        return Customisation::Error::NOT_EXISTING;
    }

    return Customisation::Error::NONE;
}

}  // namespace DNN
}  // namespace VPP
