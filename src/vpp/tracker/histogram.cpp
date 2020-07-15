/**
 *
 * @file      vpp/tracker/histogram.cpp
 *
 * @brief     This is the VPP histogram tracker implementation file
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
#include <opencv2/video/tracking.hpp>

#include "vpp/tracker/histogram.hpp"
#include "vpp/log.hpp"

namespace VPP {
namespace Tracker {
namespace Histogram {

bool Parameters::operator == (const Parameters &other) 
    const noexcept {

    return (mode     == other.mode)     && (entries  == other.entries) &&
           (storage  == other.storage)  && (sizes    == other.sizes)   &&
           (channels == other.channels);
}

Context::Context(Zone &zone, Zone::Copier &copier, unsigned int sz,
                 Parameters &params) noexcept
    : VPP::Tracker::Context(zone, copier, sz), signature(), mask(), 
      validity(1.0), config(params) {}

void Context::initialise(View &view) noexcept {
    /* Use the cached histogram mode view and only make it to the zone if 
     * it is not available */
    auto roi = std::move(view.image(config.mode, zone(-1)));

    if (config.mask.valid) {
        inRange(roi.input(), config.mask.low, config.mask.high, mask);
    } else {
        mask = cv::Mat();
    }

    cv::calcHist(&roi.input(), 1, config.channels.data(), mask, signature, 
                 config.entries, config.sizes.data(), config.ranges.data(),
                 true, false);
    cv::normalize(signature, signature, 0, 255, cv::NORM_MINMAX);
}

double Context::compare(Context &other, enum cv::HistCompMethods method)
    const noexcept {

    ASSERT(config == other.config, 
           "Engine::Context::compare(): Comparing histograms of different "
           "configuations!");

    return cv::compareHist(signature, other.signature, method);
}

void Context::camshift(View &view, cv::TermCriteria &term,
                       float threshold) noexcept {
    if (zone().valid()) {
        /* Compute the projection with the current signature */
        auto dest = std::move(back_project(view));

        /* Flush the original signature for comparisons */
        auto orig_signature = std::move(signature);

        /* Get the new histogram signature of the same location with the new
         * view, and compute an correlation histogram score */
        initialise(view);
        auto keep_score = cv::compareHist(signature, orig_signature, 
                                          cv::HISTCMP_CORREL);
        auto keep_signature = std::move(signature);

        /* Proceed with the CamShift and compute the correlation histogram
         * score with the CamShift result */
        cv::Rect estimated(zone(-1));
        cv::CamShift(dest, estimated, term);
        auto &z = stack(zone(-1));
        static_cast<cv::Rect &>(z) = estimated;
        signature = std::move(cv::Mat());
        initialise(view);
        auto shift_score = cv::compareHist(signature, orig_signature, 
                                           cv::HISTCMP_CORREL);

        /* Keep only the highest score and update the signature accordingly.
         * If both scores are below threshold, then keep the original position
         * as the tracking is flawed obviously */
        float score;
        if ((shift_score > keep_score) && (shift_score > threshold) ) {
            score = shift_score;
            z.deproject(view);
            zone(-2) = std::move(z);
            zones.pop_back();
        } else {
            score = keep_score;
            zones.pop_back();
        }
           
        signature = std::move(orig_signature);

        /* Apply the score factor to the current validity */
        validity *= score;
    }
}

cv::Mat Context::back_project(View &view) const noexcept {
    /* Get a reference to a cached view of the requested mode (and create the
     * cache if it does not exists yet). And do the back project computation
     * with this cached image */
    auto &img = view.image(config.mode);
    cv::Mat dst;

    cv::calcBackProject(&img.input(), 1, config.channels.data(), signature, dst,
                        config.ranges.data(), 1.0, true);

    /* Copy elision happens here */
    return dst;
}

Engine::Ranges::Ranges() noexcept : Customisation::Entity("Channel") {
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

Customisation::Error Engine::Ranges::setup() noexcept {
    std::vector<float> l = low;
    std::vector<float> h = high;

    /* Check if the size is correct */
    if ( (l.size() != h.size()) ) {
        LOGE("Tracker::Engine::Ranges::setup(): low and high vectors "
             "are of different sizes!");
        return Customisation::Error::INVALID_RANGE;
    }

    /* The two vectors are of same size */
    auto hit = h.begin();
    for (auto lit = l.begin(); lit != l.end(); ) {
        if (*lit > *hit) {
            LOGE("Tracker::Engine::Ranges::setup(): low boundary %f is "
                 "higher than the corresponding high boundary %f!", *lit, *hit);
            return Customisation::Error::INVALID_RANGE;
        }

        ++lit;
        ++hit;
    }

    return Customisation::Error::NONE;
} 

Engine::Engine(const Zone::Copier &c, unsigned int sz) noexcept
    : VPP::Tracker::Engine<Engine, Context>(c, sz), config() {
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

Customisation::Error Engine::setup() noexcept {
    /* Store the channel ordering directly in the common parameters */
    config.channels = channels;
    auto entries = config.channels.size();

    if (entries == 0) {
        LOGE("Tracker::Engine::setup(): no channels selected!");
        return Customisation::Error::INVALID_RANGE;
    }

    /* Extract the channels mode and keep only the channel indexes */
    int cfg = 0;
    for (auto &v : config.channels) {
        cfg |= v;
        v  = Image::Channel(v).id(); 
    }
    auto mode = Image::Channel::mode(cfg);

    switch (mode) {
        case 0:
            LOGE("Tracker::Engine::setup(): Ambiguous colour space "
                 "requested! Cannot get it from the provided channels.");
            return Customisation::Error::INVALID_VALUE;
            break;
        case Image::Mode::BGR:
        case Image::Mode::HSV:
        case Image::Mode::YUV:
        case Image::Mode::YCrCb:
            break;
        case Image::Mode::GRAY:
            if (Image::Channel::id(cfg) != 0) {
                LOGE("Tracker::Engine::setup(): Requesting channel "
                     "other than the luminance for a grayscale image!");
                return Customisation::Error::INVALID_VALUE;
            }
            break;
        default:
            LOGE("Tracker::Engine::setup(): Mixing channels from "
                 "different color spaces!");
            return Customisation::Error::INVALID_VALUE;
            break;
    }
    
    /* Store the channels mode, number of selected channels and number of bins
     * in the common parameters */
    config.mode    = mode;
    config.entries = entries;
    config.sizes   = bins;

    if (config.sizes.size() < entries) {
        LOGE("Tracker::Engine::setup(): Only %d bins have been defined "
             "despite having %d channels selected!",
             static_cast<int>(config.sizes.size()),
             static_cast<int>(entries));
        return Customisation::Error::INVALID_VALUE;
    }

    /* Create the actual histogram ranges */
    std::vector<float> low  = ranges.low;
    std::vector<float> high = ranges.high;
    
    if ((low.size() < entries) || (high.size() < entries)) {
        LOGE("Tracker::Engine::setup(): Only %d histogram ranges have "
             "been defined despite having %d channels selected!",
             static_cast<int>(std::min<unsigned int>(low.size(), high.size())),
             static_cast<int>(entries));
        return Customisation::Error::INVALID_VALUE;
    }

    /* Store all ranges in the vector and build the configuration ranges */
    config.ranges.clear();
    config.storage.clear();
    /* reserve storage vector size to ensure the buffer is *always* valid */
    config.storage.reserve(entries*2);
    auto low_it  = low.begin();
    auto high_it = high.begin();
    for (unsigned int e = 0; e < entries; e++) {
        config.storage.emplace_back(*low_it);
        config.ranges.emplace_back(&config.storage.back());
        /* Upper side is exclusive */
        config.storage.emplace_back(*high_it + 1); 
        ++low_it;
        ++high_it;
    }

    /* Fill in the mask configuration (if any) */
    low  = mask.low;
    high = mask.high;
    if ((low.size() == 0) || (high.size() == 0)) {
        config.mask.valid = false;
        return Customisation::Error::NONE;
    }
    
    config.mask.valid = true;
    if (mode == Image::Mode::GRAY) {
        if (low.size() != 1) {
            LOGE("Tracker::Engine::setup(): Expected a single component "
                 "mask vector for gray image but have a %d-component vector!",
                 static_cast<int>(low.size()));
            return Customisation::Error::INVALID_VALUE;
        }
        config.mask.low  = cv::Scalar(low[0]);
        config.mask.high = cv::Scalar(high[0]);
    } else {
        if (low.size() != 3) {
            LOGE("Tracker::Engine::setup(): Expected a 3-component mask "
                 "vector for colour image but have a %d-component vector!",
                 static_cast<int>(low.size()));
            return Customisation::Error::INVALID_VALUE;
        }
        config.mask.low  = cv::Scalar(low[0],  low[1],  low[2]);
        config.mask.high = cv::Scalar(high[0], high[1], high[2]);
    }

    return clear();
}

Customisation::Error Engine::clear() noexcept {
    storage.clear();
    return Customisation::Error::NONE;
}

void Engine::prepare(Zones &zs) noexcept {
    Parent::prepare(zs, config);
}

}  // namespace Histogram
}  // namespace Tracker
}  // namespace VPP
