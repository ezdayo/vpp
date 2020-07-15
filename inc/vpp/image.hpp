/**
 *
 * @file      vpp/image.hpp
 *
 * @brief     This is the VPP image description file
 *
 * @details   This file describes the structure of an image and adds some
 *            functions to handle the images
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

#include <opencv2/core/core.hpp>
#include "vpp/log.hpp"

namespace VPP {

class Image final {
    public:
        class Mode final {
            public:
                static constexpr int AMBIGUOUS = 0x00;
                static constexpr int BGR       = 0x01;
                static constexpr int HSV       = 0x02;
                static constexpr int YUV       = 0x04;
                static constexpr int YCrCb     = 0x08;
                static constexpr int GRAY      = 0x10;
                static constexpr int DEPTH16   = 0x20;
                static constexpr int DEPTHF    = 0x40;
                static constexpr int MOTION    = 0x80;
                static constexpr int MASK      = 0xFF;

                /* Get the number of channels in a given mode */
                inline static int channels(int m) noexcept {
                    switch(m) {
                        /* 3-channel modes */
                        case BGR:
                        case HSV:
                        case YUV:
                        case YCrCb:
                        /* Ambiguous can be 3-channel images such as YUV */
                        case AMBIGUOUS:
                            return 3;

                        /* Dual channel mode */
                        case MOTION:
                            return 2;

                        /* Single channel modes */
                        case DEPTH16:
                        case DEPTHF:
                        case GRAY:
                            return 1;

                        /* Otherwise, the mode is broken */
                        default:
                            return 0;
                    }
                }

                inline static bool is_colour(int m) noexcept {
                    return (m == BGR) || (m == HSV) || (m == YUV) ||
                           (m == YCrCb);
                }

                inline static bool is_depth(int m) noexcept {
                    return (m == DEPTH16) || (m == DEPTHF);
                }

                inline static bool is_gray(int m) noexcept {
                    return (m == GRAY);
                }

                inline static bool is_motion(int m) noexcept {
                    return (m == MOTION);
                }


                inline Mode() noexcept : mode(AMBIGUOUS) {}

                inline Mode(int m) noexcept : mode(m) {
                    ASSERT( is_colour(m) || is_gray(m) || is_depth(m) || 
                            is_motion(m),
                           "Image::Mode::Mode(): invalid mode provided "
                           "%d!", m); 
                }

                inline Mode(const Mode& other) = default;
                inline Mode(Mode&& other) = default;
                inline Mode& operator=(const Mode& other) = default;
                inline Mode& operator=(Mode&& other) = default;

                inline ~Mode() noexcept = default;

                inline operator int() const noexcept {
                    return mode;
                }

                inline bool operator ==(int m) const noexcept {
                    return m == mode;
                }

                inline bool operator ==(const Mode &m) const noexcept {
                    return m.mode == mode;
                }

                inline bool is_colour() const noexcept {
                    return is_colour(mode);
                }

                inline bool is_depth() const noexcept {
                    return is_depth(mode);
                }

                inline bool is_gray() const noexcept {
                    return is_gray(mode);
                }

                inline bool is_motion() const noexcept {
                    return is_motion(mode);
                }
 
                inline int channels() const noexcept {
                    return channels(mode);
                }

                inline bool valid() const noexcept {
                    return channels() != 0;
                }

            private:
                int mode;
        };

        class Channel final {
            public:
                /* A handy macro to make the code easier to understand */
                #define MODE(x) (Mode::x << 4)

                /* Channel indexes are coded on bits[0:3] and modes are encoded
                 *  in bits [4:9]*/

                /* B only exists in BGR */
                static constexpr int B       = 0x000 | MODE(BGR);
                /* G only exists in BGR */
                static constexpr int G       = 0x001 | MODE(BGR);
                /* R only exists in BGR */
                static constexpr int R       = 0x002 | MODE(BGR);
                /* H only exists in HSV */
                static constexpr int H       = 0x000 | MODE(HSV);
                /* S only exists in HSV */
                static constexpr int S       = 0x001 | MODE(HSV);
                /* V exists in HSV and YUV*/
                static constexpr int V       = 0x002 | (MODE(HSV)&MODE(YUV));
                /* Y exists in YUV and YCrCb */
                static constexpr int Y       = 0x000 | (MODE(YUV)&MODE(YCrCb));
                /* U only exists in YUV */
                static constexpr int U       = 0x001 | MODE(YUV);
                /* Cr only exists in YCrCb */
                static constexpr int Cr      = 0x001 | MODE(YCrCb);
                /* Cb only exists in YCrCb */
                static constexpr int Cb      = 0x002 | MODE(YCrCb);
                /* Vx only exists in MOTION */
                static constexpr int Vx      = 0x000 | MODE(MOTION);
                /* Vy only exists in MOTION */
                static constexpr int Vy      = 0x001 | MODE(MOTION);
                /* Mask to get the channel id */
                static constexpr int ID      = 0x00F;

                /* Modes encoding in channels */
                static constexpr int BGR     = MODE(BGR);
                static constexpr int HSV     = MODE(HSV);
                static constexpr int YUV     = MODE(YUV);
                static constexpr int YCrCb   = MODE(YCrCb);
                static constexpr int GRAY    = MODE(GRAY);
                static constexpr int DEPTH16 = MODE(DEPTH16);
                static constexpr int DEPTHF  = MODE(DEPTHF);
                static constexpr int MOTION  = MODE(MOTION);
                /* Mask for getting the mode */
                static constexpr int MODE    = MODE(MASK);
                #undef MODE

                inline static int mode(int c) {
                    return ((c >> 4) & Mode::MASK);
                }

                inline static int id(int c) {
                    return (c & ID);
                }

                inline static bool valid(int c) {
                    return (c >=0) && 
                           (Channel::id(c) < Mode::channels(Channel::mode(c)));
                }

                inline explicit Channel(int c) noexcept : channel(c) {
                    ASSERT(valid(c),
                           "Image::Channel::Channel(): invalid channel "
                           "provided %d!", c); 
                }

                inline Channel(const Channel& other) = default;
                inline Channel(Channel&& other) = default;
                inline Channel& operator=(const Channel& other) = default;
                inline Channel& operator=(Channel&& other) = default;

                inline ~Channel() noexcept = default;

                inline bool in(const Mode &m) const noexcept {
                    /* Modes must be compatibles and the channel ID shall be
                     * valid */
                    return ( ((mode(channel) == Mode::AMBIGUOUS) || 
                              (mode(channel) == m)) &&
                             (id(channel) < m.channels()) );
                }

                inline Channel &on(const Mode &m) noexcept {
                    ASSERT(in(m),
                           "Image::Channel::on(): Unable to switch from "
                           "channel mode %d to channel mode %d!",
                            mode(channel), static_cast<int>(m));
                    channel |= (m << 4);

                    return *this;
                }
 
                inline bool valid() const noexcept {
                    return valid(channel);
                }

                inline int id() const noexcept {
                    return id(channel);
                }

                inline int mode() const noexcept {
                    return mode(channel);
                }

                inline operator int() const noexcept {
                    return channel;
                }

                inline bool operator ==(int c) const noexcept {
                    return c == channel;
                }

                inline bool operator ==(const Channel &c) const noexcept {
                    return c.channel == channel;
                }

            private:
                int channel;
            };

        static Image INVALID;

        /* Invalid image */
        Image() noexcept;

        /* Use those image constructors */
        explicit Image(cv::Mat d, Mode m) noexcept;
        explicit Image(const Image &i, Mode m, const cv::Rect &roi,
                       float scale = 1.0, float offset = 0.0) noexcept;
        explicit Image(const Image &i, Mode m, float scale = 1.0,
                       float offset = 0.0) noexcept;
        ~Image() noexcept;

        /* Images can be copied and moved. Note that only the original input
         * buffer is kept on copies, the output and drawable are cleared. */
        Image(const Image& other) noexcept;
        Image(Image&& other) = default;
        Image& operator=(const Image& other) noexcept;
        Image& operator=(Image&& other) = default;

        Image operator()(const cv::Rect &roi) const noexcept;

        inline bool valid() const noexcept {
            return m.valid();
        }

        inline Mode mode() const noexcept {
            return m;
        }

        inline const cv::Mat &input() const noexcept {
            return original;
        }

        const cv::Mat &output() const noexcept;

        cv::Mat &drawable() noexcept;

        inline const cv::Rect & frame() const noexcept {
            return boundaries;
        }

        void flush() noexcept;

        cv::Mat extract(const Channel &c, const cv::Rect &roi) const noexcept;
        cv::Mat extract(const Channel &c) const noexcept;

        bool translatable(const Mode &mode) const noexcept;

        /* The next all create new matrices, even in the same mode. Calling
         * to with the same mode and size creates a deep copy of the original
         * matrix */
        cv::Mat to(const Mode &mode, const cv::Rect &roi, float scale = 1.0,
                   float offset = 0.0) const noexcept;
        cv::Mat to(const Mode &mode, float scale = 1.0,
                   float offset = 0.0) const noexcept;

    private:
        Mode     m;
        cv::Rect boundaries;
        cv::Mat  original;
        cv::Mat  copy;
};

}  // namespace VPP
