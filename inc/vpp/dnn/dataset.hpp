/**
 *
 * @file      vpp/dnn/dataset.hpp
 *
 * @brief     This is the VPP DNN dataset description file
 *
 * @details   This is the definition of the VPP DNN dataset, a class that reads
 *            a dataset file and generate the related information, such as the
 *            bindings between indexes and descriptions, as well as the creation
 *            of a unique dataset index. Note that if a dataset is used in
 *            multiple DNN, then only one copy of the dataset will be present
 *            in memory.
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
#include "vpp/scene.hpp"

namespace VPP {
namespace DNN {

class Dataset : public Parametrisable {
    public:
        Dataset() noexcept;
        virtual ~Dataset() noexcept; 

        virtual Customisation::Error setup() noexcept override;
        virtual void terminate() noexcept override;

        std::string label(const Zone &zone) const noexcept;
        std::string label(const Zone &zone, float threshold) const noexcept;

        int16_t ID() const noexcept;
        int size() const noexcept;

        int16_t textID() const noexcept;
        int32_t textGID() const noexcept;

        static bool isText(const Zone &zone) noexcept;

        Customisation::File labels;

    protected:
        int16_t id;
};

}  // namespace DNN
}  // namespace VPP
