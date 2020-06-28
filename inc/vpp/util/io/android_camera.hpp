/**
 *
 * @file      vpp/util/io/android_camera.hpp
 *
 * @brief     This is the Android Camera Input class definition
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

#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include <media/NdkImageReader.h>
#include <opencv2/opencv.hpp>
#include <future>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "vpp/util/io/input.hpp"

namespace Util
{
namespace IO
{

class AndroidCamera : public Input
{
public:
    struct Sensor final
    {
        // Maximum width of the sensor in pixels
        int32_t width;
        // Maximum heigh of the sensor in pixels
        int32_t height;
        // Clockwise angle through which the output image needs to be
        // rotated to be upright on the device screen in its native
        // orientation
        int32_t orientation;
        // Lens facing orientation (0 is front, 1 is back, 2 is external)
        int32_t facing;
        // Minimal image exposure time
        int64_t minExposure;
        // Maximal image exposure time
        int64_t maxExposure;
        // Minimal sensitivity as defined in ISO 12232:2006
        int32_t minSensitivity;
        // Maximal sensitivity as defined in ISO 12232:2006
        int32_t maxSensitivity;
        //max Frame duration
        int64_t maxFrameDuration;
    };

    struct Mode final
    {
        // Supported format
        int32_t format;
        // Maximum width for the format in pixels
        int32_t width;
        // Maximum heigh for the format in pixels
        int32_t height;

        Mode(int32_t f, int32_t w, int32_t h) noexcept
            : format(std::move(f)), width(std::move(w)),
              height(std::move(h)) {}

        bool operator==(const Mode &other) const noexcept
        {
            return ((format == other.format) &&
                    (width == other.width) &&
                    (height == other.height));
        }

        void swapWidthHeight() noexcept
        {
            std::swap(width, height);
        }

        Mode swapWidthHeightIf(bool doSwap) noexcept
        {
            if (doSwap)
                return Mode(format, height, width);
            else
                return Mode(format, width, height);
        }
    };

    struct ImageReaderCallback final
    {
        std::promise<int> *promise;
        cv::Mat *image;
    };

    AndroidCamera() noexcept;
    virtual ~AndroidCamera() noexcept;

    AndroidCamera(const AndroidCamera &other) = delete;
    AndroidCamera(AndroidCamera &&other) = delete;
    AndroidCamera &operator=(const AndroidCamera &other) = delete;
    AndroidCamera &operator=(AndroidCamera &&other) = delete;

    virtual std::vector<std::string> sources() const noexcept override;
    virtual std::vector<std::string> modes() noexcept override;

    virtual int open(const std::string &protocol, int id) noexcept override;
    int open(int id) noexcept;

    virtual int open(const std::string &protocol,
                     const std::string &source) noexcept override;
    int open(const std::string &camera) noexcept;

    virtual int setup(const std::string &username,
                      const std::string &password) noexcept override;

    int setup(int &width, int &height) noexcept;
    virtual int setup(int &width, int &height, int &rotation) noexcept override;
    int setup(Mode mode) noexcept;
    int setup(Mode mode, int screenOrientation) noexcept;

    virtual int read(cv::Mat &image, VPP::Image::Mode &mode) noexcept override;

    const Sensor &sensor() const noexcept;
    const Mode &mode() const noexcept;
    const std::vector<Mode> &capabilities() const noexcept;

    virtual int close() noexcept override;

private:
    void unsetup() noexcept;

    ACameraManager *cameraManager;
    std::vector<std::string> cameraNames;
    std::string cameraName;
    ACameraDevice *cameraDevice;
    ACameraDevice_stateCallbacks cameraDeviceCallbacks;
    Sensor cameraSensor;
    std::vector<Mode> captureCapabilities;
    ACaptureRequest *captureRequest;
    Mode captureMode;
    int imageRotation;
    Mode imageMode;
    AImageReader *imageReader;
    AImageReader_ImageListener imageListener;
    std::queue<ImageReaderCallback> imageReaderCallbacks;
    ANativeWindow *imageWindow;
    ACameraOutputTarget *imageTarget;
    ACaptureSessionOutputContainer *imageOutputs;
    ACaptureSessionOutput *imageOutput;
    ACameraCaptureSession *captureSession;
    ACameraCaptureSession_stateCallbacks captureStateCallbacks;
};

} // namespace IO
} // namespace Util
