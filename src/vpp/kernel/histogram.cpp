/**
 *
 * @file      vpp/kernel/histogram.cpp
 *
 * @brief     This is the VPP histogram kernel implementation file
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

#include <algorithm>
#include <functional>
#include <opencv2/core/mat.hpp>

#include "vpp/kernel/histogram.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace Kernel {

bool Histogram::Parameters::operator == (const Parameters &other) 
    const noexcept {

    return (histogram.mode     == other.histogram.mode) &&
           (histogram.entries  == other.histogram.entries) &&
           (histogram.storage  == other.histogram.storage) && 
           (histogram.sizes    == other.histogram.sizes) &&
           (histogram.channels == other.histogram.channels);
}

Histogram::Data::Data(Histogram::Parameters &params, View &view,
                      const cv::Rect &zone) noexcept
    : config(params) {
    update(view, zone);
}

Histogram::Data::~Data() noexcept = default;

void Histogram::Data::prepare_input(View &view, 
                                    const Image::Mode &mode) noexcept {
    view.cache(mode);
}

void Histogram::Data::update(View &view, const cv::Rect &zone) noexcept {
    /* Use the cached histogram mode view and only make it to the zone if 
     * it is not available */
    auto roi = std::move(view.image(config.histogram.mode, zone));

    if (config.mask.valid) {
        inRange(roi.input(), config.mask.low, config.mask.high, mask);
    } else {
        mask = cv::Mat();
    }

    cv::calcHist(&roi.input(), 1, config.histogram.channels.data(), mask,
                 signature, config.histogram.entries, 
                 config.histogram.sizes.data(), config.histogram.ranges.data(),
                 true, false);
    cv::normalize(signature, signature, 0, 255, cv::NORM_MINMAX);
}

double Histogram::Data::compare(Data &other, enum cv::HistCompMethods method)
    const noexcept {

    ASSERT(config == other.config, 
           "Histogram::Data::compare(): Comparing histograms of different "
           "configuations!");

    return cv::compareHist(signature, other.signature, method);
}

cv::Mat Histogram::Data::back_project(View &view) const noexcept {
    /* Get a reference to a cached view of the requested mode (and create the
     * cache if it does not exists yet). And do the back project computation
     * with this cached image */
    auto &img = view.image(config.histogram.mode);
    cv::Mat dst;

    cv::calcBackProject(&img.input(), 1, config.histogram.channels.data(),
                        signature, dst, config.histogram.ranges.data(), 1.0, 
                        true);

    /* Copy elision happens here */
    return dst;
}

Histogram::Ranges::Ranges() noexcept : Customisation::Entity("Channel") {
    low.denominate("low")
       .describe("The inclusive dynamic low values for each channel. Pixels "
                 "having a value stricly lower than this low boundary are "
                 "masked out")
       .characterise(Customisation::Trait::CONFIGURABLE);
    low.range(0, 255);
    Customisation::Entity::expose(low);

    high.denominate("high")
        .describe("The inclusive dynamic high values for each channel. Pixels "
                  "having a value stricly higher than this high boundary are "
                  "masked out")
        .characterise(Customisation::Trait::CONFIGURABLE);
    high.range(0, 255);
    Customisation::Entity::expose(high);
}

Histogram::Ranges::~Ranges() noexcept = default;

Customisation::Error Histogram::Ranges::setup() noexcept {
    std::vector<float> l = low;
    std::vector<float> h = high;

    /* Check if the size is correct */
    if ( (l.size() != h.size()) ) {
        LOGE("Kernel::Histogram::Ranges::setup(): low and high vectors "
             "are of different sizes!");
        return Customisation::Error::INVALID_RANGE;
    }

    /* The two vectors are of same size */
    auto hit = h.begin();
    for (auto lit = l.begin(); lit != l.end(); ) {
        if (*lit > *hit) {
            LOGE("Kernel::Histogram::Ranges::setup(): low boundary %f is "
                 "higher than the corresponding high boundary %f!", *lit, *hit);
            return Customisation::Error::INVALID_RANGE;
        }

        ++lit;
        ++hit;
    }

    return Customisation::Error::NONE;
} 

Histogram::Histogram() noexcept
    : Customisation::Entity("Channel"), tracked(), config() {
    channels.denominate("channels")
            .describe("The selected channels for the histogram, can be any "
                      "combination of H, S, V or R, G, B or GRAY -"
                      "But not a mix of these!")
            .characterise(Customisation::Trait::CONFIGURABLE);
    channels.define("B",        Image::Channel::B);
    channels.define("B:BGR",    Image::Channel::B|Image::Channel::BGR);
    channels.define("G",        Image::Channel::G);
    channels.define("G:BGR",    Image::Channel::G|Image::Channel::BGR);
    channels.define("R",        Image::Channel::R);
    channels.define("R:BGR",    Image::Channel::R|Image::Channel::BGR);
    channels.define("H",        Image::Channel::H);
    channels.define("H:HSV",    Image::Channel::H|Image::Channel::HSV);
    channels.define("S",        Image::Channel::S);
    channels.define("S:HSV",    Image::Channel::S|Image::Channel::HSV);
    channels.define("V",        Image::Channel::V);
    channels.define("V:HSV",    Image::Channel::V|Image::Channel::HSV);
    channels.define("Y",        Image::Channel::Y);
    channels.define("Y:YUV",    Image::Channel::Y|Image::Channel::YUV);
    channels.define("U",        Image::Channel::U);
    channels.define("U:YUV",    Image::Channel::U|Image::Channel::YUV);
    channels.define("V",        Image::Channel::V);
    channels.define("V:YUV",    Image::Channel::V|Image::Channel::YUV);
    channels.define("Y:YCrCb",  Image::Channel::Y|Image::Channel::YCrCb);
    channels.define("Cr",       Image::Channel::Cr);
    channels.define("Cr:YCrCb", Image::Channel::Cr|Image::Channel::YCrCb);
    channels.define("Cb",       Image::Channel::Cb);
    channels.define("Cb:YCrCb", Image::Channel::Cb|Image::Channel::YCrCb);
    channels.define("GRAY",     Image::Channel::GRAY);

    Customisation::Entity::expose(channels);
    
    mask.denominate("mask")
        .describe("The inclusive dynamic ranges of the mask for all image "
                  "channels, i.e. BGR if the selected channels are BGR ones, "
                  "YUV if the selected channels are BGR ones, or a gray range "
                  "otherwise")
        .characterise(Customisation::Trait::CONFIGURABLE);
    Customisation::Entity::expose(ranges);
    
    ranges.denominate("ranges")
          .describe("The inclusive dynamic ranges for all selected channels, "
                    "i.e. only the channels that are selected in channels")
          .characterise(Customisation::Trait::CONFIGURABLE);
    Customisation::Entity::expose(ranges);

    bins.denominate("bins")
        .describe("The number of buckets to quantize each channel in the "
                  "histogram, i.e. for each selected channel in channels")
        .characterise(Customisation::Trait::CONFIGURABLE);
    bins.range(2, 256);
    Customisation::Entity::expose(bins);

    /* Setup a default configuration */
    channels    = { Image::Channel::H, Image::Channel::S, Image::Channel::V };
    mask.low    = std::vector<float>(); /* Empty vector */
    mask.high   = std::vector<float>(); /* Empty vector */
    ranges.low  = { 0, 0, 0 };
    ranges.high = { 179, 255, 255 };
    bins        = { 180, 256, 256 };
}

Histogram::~Histogram() noexcept = default;

Customisation::Error Histogram::setup() noexcept {
    /* Store the channel ordering directly in the common parameters */
    config.histogram.channels = channels;
    auto entries = config.histogram.channels.size();

    if (entries == 0) {
        LOGE("Kernel::Histogram::setup(): no channels selected!");
        return Customisation::Error::INVALID_RANGE;
    }

    /* Extract the channels mode and keep only the channel indexes */
    int c = 0;
    for (auto &v : config.histogram.channels) {
        c |= v;
        v  = Image::Channel(v).id(); 
    }
    auto cfg = Image::Channel(c);
    auto mode = cfg.mode();

    switch (mode) {
        case 0:
            LOGE("Kernel::Histogram::setup(): Ambiguous colour space "
                 "requested! Cannot get it from the provided channels.");
            return Customisation::Error::INVALID_VALUE;
            break;
        case Image::Mode::BGR:
        case Image::Mode::HSV:
        case Image::Mode::YUV:
        case Image::Mode::YCrCb:
            break;
        case Image::Mode::GRAY:
            if ((cfg.id()) != 0) {
                LOGE("Kernel::Histogram::setup(): Requesting channel "
                     "other than the luminance for a grayscale image!");
                return Customisation::Error::INVALID_VALUE;
            }
            break;
        default:
            LOGE("Kernel::Histogram::setup(): Mixing channels from "
                 "different color spaces!");
            return Customisation::Error::INVALID_VALUE;
            break;
    }
    
    /* Store the channels mode, number of selected channels and number of bins
     * in the common parameters */
    config.histogram.mode    = mode;
    config.histogram.entries = entries;
    config.histogram.sizes   = bins;
    
    if (config.histogram.sizes.size() < entries) {
        LOGE("Kernel::Histogram::setup(): Only %d bins have been defined "
             "despite having %d channels selected!",
             static_cast<int>(config.histogram.sizes.size()),
             static_cast<int>(entries));
        return Customisation::Error::INVALID_VALUE;
    }

    /* Create the actual histogram ranges */
    std::vector<float> low  = mask.low;
    std::vector<float> high = mask.high;
    
    if (low.size() < entries) {
        LOGE("Kernel::Histogram::setup(): Only %d histogram ranges have "
             "been defined despite having %d channels selected!",
             static_cast<int>(low.size()), static_cast<int>(entries));
        return Customisation::Error::INVALID_VALUE;
    }

    /* Store all ranges in the vector and build the configuration ranges */
    config.histogram.ranges.clear();
    config.histogram.storage.clear();
    // to ensure the buffer is *always* valid
    config.histogram.storage.reserve(low.size()*2);
    auto high_it = high.begin();
    for (auto low_it = low.begin(); low_it != low.end(); ) {
        config.histogram.storage.emplace_back(*low_it);
        config.histogram.ranges.emplace_back(&config.histogram.storage.back());
        /* Upper side is exclusive */
        config.histogram.storage.emplace_back(*high_it + 1); 
        ++low_it;
        ++high_it;
    }

    /* Fill in the mask configuration (if any) */
    low = mask.low;
    if (low.size() == 0) {
        config.mask.valid = false;
        return Customisation::Error::NONE;
    }
    
    config.mask.valid = true;
    if (mode == Image::Mode::GRAY) {
        if (low.size() != 1) {
            LOGE("Kernel::Histogram::setup(): Expected a single component "
                 "mask vector for gray image but have a %d-component vector!",
                 static_cast<int>(low.size()));
            return Customisation::Error::INVALID_VALUE;
        }
        config.mask.low  = cv::Scalar(low[0]);
        config.mask.high = cv::Scalar(high[0]);
    } else {
        if (low.size() != 3) {
            LOGE("Kernel::Histogram::setup(): Expected a 3-component mask "
                 "vector for colour image but have a %d-component vector!",
                 static_cast<int>(low.size()));
            return Customisation::Error::INVALID_VALUE;
        }
        config.mask.low  = cv::Scalar(low[0],  low[1],  low[2]);
        config.mask.high = cv::Scalar(high[0], high[1], high[2]);
    }

    return Customisation::Error::NONE;
}

Histogram::Data &Histogram::data(View &view, const Zone &zone) noexcept {
    auto key = zone.uuid;
    auto it = tracked.find(key);

    if (it == tracked.end()) {
        auto p = tracked.emplace(std::piecewise_construct,
                                 std::forward_as_tuple(key), 
                                 std::forward_as_tuple(config, view, zone));
        it = p.first;
    }

    return it->second;
}

}  // namespace Kernel
}  // namespace VPP