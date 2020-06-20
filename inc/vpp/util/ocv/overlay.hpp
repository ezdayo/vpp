/**
 *
 * @file      vpp/util/ocv/overlay.hpp
 *
 * @brief     This is a versatile OpenCV overlay class
 *
 * @details   This is the definition of a base class for displaying visual
 *            information on an OCV frame using a common style. It allows to
 *            draw boxes of all kinds, put text anywhere and overlay images.
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
#include <opencv2/imgproc.hpp>

namespace Util {
namespace OCV {

class Overlay {

    public:
        enum class AAMode : int {
            LINE_8  = cv::LINE_8,
            LINE_4  = cv::LINE_4,
            LINE_AA = cv::LINE_AA,
        };

        struct DrawingStyle final {
            int        thickness;
            AAMode     antialiasing;
            cv::Scalar color;
        };

        struct LayerStyle final {
            double saturation;
        };

        class Font {
            public:
                virtual ~Font() noexcept;

                /* Name and location of the font */
                const std::string name;
                const std::string location;

                /* Getting a Font object using a static factory method */
                static Font *any() noexcept;
                static Font *use(const std::string &name) noexcept;
                static Font *use(const std::string &name,
                                 const std::string &path) noexcept;

                /* Drawing some text */
                virtual void write(cv::Mat &frame, const std::string &text,
                                   const cv::Point &at, int thickness,
                                   AAMode antialiasing, cv::Scalar color,
                                   int height) const noexcept;

            protected:
                explicit Font(std::string id, std::string path) noexcept;
        };
        
        struct TextStyle final {
            int        thickness;
            AAMode     antialiasing;
            cv::Scalar color;
            int        height;
            Font *     font;
        };

        class Layer {
            public:
                Layer() noexcept;
                ~Layer() noexcept;

                void clear() noexcept;
                bool empty() const noexcept;
                void merge(cv::Mat &frame, const cv::Point &at,
                           const LayerStyle &style) const noexcept;
                void set(const std::string &filename) noexcept;
                void set(cv::Size size, const uint8_t *bgr,
                         const uint8_t *alpha) noexcept;
                void set(const cv::Mat &bgr, const cv::Mat &alpha) noexcept;
    
                int width;
                int height;

            private:
                cv::Mat fg, msk;
        };


        Overlay() noexcept;
        ~Overlay() noexcept;

        /* Resetting default styles */
        void resetDefaultDrawingStyle() noexcept;
        void resetDefaultLayerStyle() noexcept;
        void resetDefaultTextStyle() noexcept;

        /* Box drawing primitives */
        void draw(cv::Mat &frame, const cv::Size &box) const noexcept;
        void draw(cv::Mat &frame, const cv::Size &box, const cv::Point &at)
            const noexcept;
        void draw(cv::Mat &frame, const cv::Size &box, const cv::Point &at,
                  const DrawingStyle &style) const noexcept;

        void draw(cv::Mat &frame, const cv::Rect &box) const noexcept;
        void draw(cv::Mat &frame, const cv::Rect &box,
                  const DrawingStyle &style) const noexcept;

        /* Layer drawing primitives */
        void draw(cv::Mat &frame, const Layer &layer)
            const noexcept;
        void draw(cv::Mat &frame, const Layer &layer, const cv::Point &at)
            const noexcept;
        void draw(cv::Mat &frame, const Layer &layer, const cv::Point &at,
                  const LayerStyle &style) const noexcept;

        /* Text drawing primitives */
        void draw(cv::Mat &frame, const std::string &text) const noexcept;
        void draw(cv::Mat &frame, const std::string &text, const cv::Point &at)
            const noexcept;
        void draw(cv::Mat &frame, const std::string &text, const cv::Point &at,
                  const TextStyle &style) const noexcept;

        /* Default styles */
        DrawingStyle defaultDrawingStyle;
        LayerStyle   defaultLayerStyle;
        TextStyle    defaultTextStyle;
};

}  // namespace OCV
}  // namespace Util
