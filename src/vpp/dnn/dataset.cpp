/**
 *
 * @file      vpp/dnn/dataset.cpp
 *
 * @brief     This is the VPP DNN dataset implementation file
 *
 * @details   This is the implementation of the VPP DNN dataset.
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

#include <vector>

#include "vpp/dnn/dataset.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace DNN {

struct DatasetCore {
        std::string              path;
        std::vector<std::string> classes;
        int16_t                  text_id;
};

static std::vector<DatasetCore> datasets = {};

Dataset::Dataset() noexcept
    : Customisation::Entity("Dataset"), labels(""), id(-1) {
        labels.denominate("labels")
              .describe("The configuration file for the dataset labels")
              .characterise(Customisation::Trait::CONFIGURABLE);
        expose(labels);
}

Dataset::~Dataset() noexcept {
    terminate();
} 

Customisation::Error Dataset::setup() noexcept {
    int16_t cur_id = 0;
    std::string requested = labels;

    for (const auto &ds : datasets) {
        if (ds.path == requested) {
            id = cur_id;
            return Customisation::Error::NONE;
        }
        cur_id ++;
    }

    if (!labels.exists()) {
        LOGE("%s[%s]::setup(): Cannot find dataset '%s'!", 
             value_to_string().c_str(), name().c_str(), requested.c_str());
        return Customisation::Error::NOT_EXISTING;
    }

    datasets.emplace_back(DatasetCore({ requested, {}, -1 }));
    auto &ds = datasets.back();

    id = cur_id;

    int text_id = 0;
    std::ifstream ifs(requested);
    std::string line;
    while (std::getline(ifs, line)) {
        if (line.find("text") != std::string::npos) {
            ds.text_id = text_id;
        }
        ++ text_id;
        ds.classes.push_back(line);
    }
 
    return Customisation::Error::NONE;
}

void Dataset::terminate() noexcept {
    id = -1;
}

std::string Dataset::label(const Zone &zone) const noexcept {
    return label(zone, 0.0);
}

std::string Dataset::label(const Zone &zone, float threshold) const noexcept {
    std::string desc;

    if (id >= 0) {
        ASSERT(id < (int) datasets.size(),
               "%s[%s]::label(): "
               "Invalid dataset id %d for a %d-entry dataset list",
               value_to_string().c_str(), name().c_str(),
               id, (int) datasets.size());
 
        auto ds = datasets[id];
        if (!ds.classes.empty()) {
            for (auto &prediction : zone.predictions) {
                if ((prediction.dataset != id) ||
                    (prediction.score) < threshold) {
                    continue;
                }
                auto classId = prediction.id;
                ASSERT(classId < static_cast<int>(ds.classes.size()),
                       "%s[%s]::label(): "
                       "Invalid class id %d provided for a %d-class dataset",
                       value_to_string().c_str(), name().c_str(),
                       classId, (int)ds.classes.size());
                if (!desc.empty()) desc+="|";
                desc += ds.classes[classId];
            }
        }
    }

    int cm = zone.tracked.centre.z *100;
    if (cm > 0) {
        desc += " @ " + std::to_string(cm/100) + "." + 
                std::to_string(cm%100) + "m";
    }

    return desc;
}
        
int16_t Dataset::ID() const noexcept {
    return id;
}

int Dataset::size() const noexcept {
    if (id >= 0) {
        ASSERT(id < (int) datasets.size(),
               "%s[%s]::size(): "
               "Invalid dataset id %d for a %d-entry dataset list",
               value_to_string().c_str(), name().c_str(),
               id, (int) datasets.size());
        return datasets[id].classes.size();
    }

    return 0;
}

int16_t Dataset::textID() const noexcept {
    if (id >= 0) {
        ASSERT(id < (int) datasets.size(),
               "%s[%s]::textID(): "
               "Invalid dataset id %d for a %d-entry dataset list",
               value_to_string().c_str(), name().c_str(),
               id, (int) datasets.size());
        return datasets[id].text_id;
    }

    return -1;
}

int32_t Dataset::textGID() const noexcept {
    return Prediction::gid(id, textID());
}

bool Dataset::isText(const Zone &zone) noexcept {
    auto dataset_id = zone.context.dataset;

    if (dataset_id >= 0) {
        ASSERT(dataset_id < (int) datasets.size(),
               "Dataset::isText(): "
               "Invalid dataset id %d for a %d-entry dataset list",
               dataset_id, (int) datasets.size());

        return (zone.context.id == datasets[dataset_id].text_id);
    }

    return false;
}

}  // namespace DNN
}  // namespace VPP
