/**
 *
 * @file      vpp/util/io/android_camera.cpp
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

#include <cstring>
#include "vpp/log.hpp"
#include "vpp/util/io/android_camera.hpp"

namespace Util
{
namespace IO
{

static void onDisconnected(void *context, ACameraDevice * /*device*/)
{
    AndroidCamera *cam = static_cast<AndroidCamera *>(context);

    LOGD("AndroidCamera::onDisconnected()");
    cam->close();
}

static void onError(void *context, ACameraDevice * /*device*/, int error)
{
    AndroidCamera *cam = static_cast<AndroidCamera *>(context);

    LOGE("AndroidCamera::onError(): Camera error %d", error);
    cam->close();
}

static void onSessionActive(void * /*context*/,
                            ACameraCaptureSession * /*session*/)
{
    LOGD("AndroidCamera::onSessionActive()");
}

static void onSessionReady(void * /*context*/,
                           ACameraCaptureSession * /*session*/)
{
    LOGD("AndroidCamera::onSessionReady()");
}

static void onSessionClosed(void * /*context*/,
                            ACameraCaptureSession * /*session*/)
{
    LOGD("AndroidCamera::onSessionClosed()");
}

static void onImageAvailable(void *context, AImageReader *imageReader)
{
    LOGD("AndroidCamera::onImageAvailable()");

    auto callbacks =
        static_cast<std::queue<AndroidCamera::ImageReaderCallback> *>(context);
    ASSERT(!callbacks->empty(), "No callback for onImageAvailable");

    // Get the first callback
    auto callback = callbacks->front();
    callbacks->pop();

    // Get their promise and image
    auto promise = callback.promise;
    auto image = callback.image;

    // Get the AImage structure
    AImage *aImage;
    auto media_error = AImageReader_acquireNextImage(imageReader, &aImage);
    if (media_error != AMEDIA_OK)
    {
        if (media_error == AMEDIA_ERROR_INVALID_PARAMETER)
        {
            promise->set_value(ACAMERA_ERROR_INVALID_PARAMETER);
        }
        else if (media_error == AMEDIA_IMGREADER_NO_BUFFER_AVAILABLE)
        {
            promise->set_value(ACAMERA_ERROR_NOT_ENOUGH_MEMORY);
        }
        else
        {
            promise->set_value(ACAMERA_ERROR_UNKNOWN);
        }
        return;
    }

    // Launch a new thread not to block the callback for too long
    std::thread processor([=]() {
        int error = ACAMERA_OK;

        // Read the image and make it an OpenCV matrix
        int32_t format = 0;
        AImage_getFormat(aImage, &format);

        if (format == AIMAGE_FORMAT_YUV_420_888)
        {
            uint8_t *yd, *ud, *vd;
            int yl, ul, vl;
            int32_t ys, us, vs;
            if ((AImage_getPlaneData(aImage, 0, &yd, &yl) != AMEDIA_OK) ||
                (AImage_getPlaneData(aImage, 1, &ud, &ul) != AMEDIA_OK) ||
                (AImage_getPlaneData(aImage, 2, &vd, &vl) != AMEDIA_OK) ||
                (AImage_getPlaneRowStride(aImage, 0, &ys) != AMEDIA_OK) ||
                (AImage_getPlaneRowStride(aImage, 1, &us) != AMEDIA_OK) ||
                (AImage_getPlaneRowStride(aImage, 2, &vs) != AMEDIA_OK))
            {
                error = ACAMERA_ERROR_PERMISSION_DENIED;
            }
            else
            {
                int32_t width, height, uvsz;
                AImage_getWidth(aImage, &width);
                AImage_getHeight(aImage, &height);
                cv::ColorConversionCodes convMode;

                uvsz = width * height / 4;

                cv::Mat yi(height, width, CV_8UC1, yd, ys);
                cv::Mat yuv(3 * height / 2, width, CV_8UC1);
                auto data = yuv.ptr();
                cv::Mat yo(height, width, CV_8UC1, data);
                yi.copyTo(yo);

                if (ud == (vd + 1))
                {
                    convMode = cv::COLOR_YUV2BGR_NV21;
                    cv::Mat uvi(height / 2, width, CV_8UC1, vd, vs);
                    cv::Mat uvo(height / 2, width, CV_8UC1, data + 4 * uvsz);
                    uvi.copyTo(uvo);
                }
                else if (vd == (ud + 1))
                {
                    convMode = cv::COLOR_YUV2BGR_NV12;
                    cv::Mat uvi(height / 2, width, CV_8UC1, ud, us);
                    cv::Mat uvo(height / 2, width, CV_8UC1, data + 4 * uvsz);
                    uvi.copyTo(uvo);
                }
                else
                {
                    convMode = cv::COLOR_YUV2BGR_I420;
                    cv::Mat ui(height / 2, width / 2, CV_8UC1, ud, us);
                    cv::Mat vi(height / 2, width / 2, CV_8UC1, vd, vs);
                    cv::Mat uo(height / 2, width / 2, CV_8UC1, data + 4 * uvsz);
                    cv::Mat vo(height / 2, width / 2, CV_8UC1, data + 5 * uvsz);
                    ui.copyTo(uo);
                    vi.copyTo(vo);
                }

                cv::cvtColor(yuv, *image, convMode, 3);
                LOGD("AndroidCamera::onImageAvailable(): "
                     "Created cv::Mat from YUV image");
            }
        }
        else if (format == AIMAGE_FORMAT_JPEG)
        {
            uint8_t *data;
            int length;
            if (AImage_getPlaneData(aImage, 0, &data, &length) != AMEDIA_OK)
            {
                error = ACAMERA_ERROR_PERMISSION_DENIED;
            }
            else
            {
                std::vector<int8_t> jpeg(data, data + length);
                *image = cv::imdecode(jpeg, cv::IMREAD_COLOR);
                LOGD("AndroidCamera::onImageAvailable(): "
                     "Created cv::Mat from JPEG image");
            }
        }
        else /* Unknown format, can this happen? */
        {
            LOGE("AndroidCamera::onImageAvailable(): "
                 "Invalid format provided 0x%x",
                 format);
            error = ACAMERA_ERROR_INVALID_PARAMETER;
        }

        // Do not forget to delete the image before leaving
        AImage_delete(aImage);
        promise->set_value(error);
    });
    processor.detach();
}

AndroidCamera::AndroidCamera() noexcept : Input({"internal"}),
                                          cameraManager(nullptr), cameraNames({}), cameraName("Invalid"),
                                          cameraDevice(nullptr), cameraDeviceCallbacks({}), cameraSensor({}),
                                          captureCapabilities({}), captureRequest(nullptr), captureMode(0, 0, 0),
                                          imageMode(0, 0, 0), imageReader(nullptr), imageReaderCallbacks(),
                                          imageWindow(nullptr), imageTarget(nullptr), imageOutputs(nullptr),
                                          imageOutput(nullptr), captureSession(nullptr)
{
    ACameraIdList *cameraIds = nullptr;

    // No camera has been opened yet, so initialise everything as a closed cam
    close();

    // Get a camera manager to list available devices
    cameraManager = ACameraManager_create();

    // List available devices and store their name, putting the back facing
    // camera first in the list
    ACameraManager_getCameraIdList(cameraManager, &cameraIds);

    bool foundBackFacing = false;
    for (auto i = 0; i < cameraIds->numCameras; ++i)
    {
        auto *id = cameraIds->cameraIds[i];

        ACameraMetadata *metadata;
        ACameraManager_getCameraCharacteristics(cameraManager, id, &metadata);

        ACameraMetadata_const_entry lensInfo = {};
        ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &lensInfo);

        auto facing =
            static_cast<acamera_metadata_enum_android_lens_facing_t>(lensInfo.data.u8[0]);

        // If a back-facing camera is found then put it first!
        if ((facing == ACAMERA_LENS_FACING_BACK) && (!foundBackFacing))
        {
            cameraNames.insert(cameraNames.begin(), id);
            foundBackFacing = true;
        }
        else
        {
            cameraNames.emplace_back(id);
        }
    }

    ACameraManager_deleteCameraIdList(cameraIds);

    // Set the default camera device callbacks
    cameraDeviceCallbacks.context = static_cast<void *>(this);
    cameraDeviceCallbacks.onDisconnected = onDisconnected;
    cameraDeviceCallbacks.onError = onError;

    // Set the imagelistener callback
    imageListener.context = static_cast<void *>(&imageReaderCallbacks);
    imageListener.onImageAvailable = onImageAvailable;

    // Set the default capture session state callbacks
    captureStateCallbacks.context = static_cast<void *>(this);
    captureStateCallbacks.onActive = onSessionActive;
    captureStateCallbacks.onReady = onSessionReady;
    captureStateCallbacks.onClosed = onSessionClosed;

    LOGD("AndroidCamera::AndroidCamera(): object created");
}

AndroidCamera::~AndroidCamera() noexcept
{
    LOGD("AndroidCamera::~AndroidCamera(): Object deleting");
    // Close any opened camera
    close();

    // Clean the camera manager resources
    ACameraManager_delete(cameraManager);
    cameraManager = nullptr;
}

std::vector<std::string> AndroidCamera::sources() const noexcept
{
    return cameraNames;
}

std::vector<std::string> AndroidCamera::modes() noexcept
{
    std::set<std::pair<int, int>> uniqueModes;
    for (auto &m : captureCapabilities)
    {
        if ((m.width >= 640) && (m.height >= 480))
        {
            uniqueModes.emplace(std::pair<int, int>(m.width, m.height));
        }
    }

    std::vector<std::string> allModes;
    for (int rotation : {0, 90, 180, 270, 360})
    {
        bool swapWidthHeight =
            (((cameraSensor.orientation + rotation + 360) % 180) != 0);
        std::string suffix;
        if (rotation > 0)
        {
            suffix = "@" + std::to_string(rotation % 360);
        }
        for (auto &m : uniqueModes)
        {
            int w = (swapWidthHeight) ? m.second : m.first;
            int h = (swapWidthHeight) ? m.first : m.second;
            allModes.emplace_back(std::to_string(w) + "x" + std::to_string(h) +
                                  suffix);
        }
    }

    return allModes;
}

int AndroidCamera::open(const std::string &protocol, int id) noexcept
{
    ASSERT(supports(protocol),
           "AndroidCamera::open(): unsupported protocol %s",
           protocol.c_str());

    if (supports(protocol))
    {
        return open(id);
    }
    else
    {
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
}

int AndroidCamera::open(int id) noexcept
{
    unsigned int cam_id;
    // Use the default back facing camera if any camera is required
    if (id < 0)
        cam_id = 0;
    else
        cam_id = static_cast<unsigned int>(id);

    if (cam_id >= cameraNames.size())
        return ACAMERA_ERROR_INVALID_PARAMETER;

    return open(cameraNames[cam_id].c_str());
}

int AndroidCamera::open(const std::string &protocol,
                        const std::string &source) noexcept
{
    ASSERT(supports(protocol),
           "AndroidCamera::open(): unsupported protocol %s",
           protocol.c_str());

    if (supports(protocol))
    {
        return open(source);
    }
    else
    {
        return ACAMERA_ERROR_INVALID_PARAMETER;
    }
}

int AndroidCamera::open(const std::string &camera) noexcept
{
    if (camera.empty())
        return ACAMERA_ERROR_INVALID_PARAMETER;
    auto name = camera.c_str();
    LOGD("AndroidCamera::open(): Opening camera '%s'", name);
    if (!cameraName.empty())
    {
        if (cameraName.compare(name) == 0)
            return ACAMERA_OK;
        close();
    }

    // Open the camera
    int error = ACameraManager_openCamera(cameraManager, name,
                                          &cameraDeviceCallbacks,
                                          &cameraDevice);
    if (error != ACAMERA_OK)
    {
        cameraDevice = nullptr;
        return error;
    }

    // Fill in the metadata from the camera
    ACameraMetadata *metadata;
    ACameraMetadata_const_entry entry;
    int32_t count = 0;
    const uint32_t *tags = nullptr;

    cameraName = name;
    LOGI("AndroidCamera::open(): Opening camera '%s'", name);

    ACameraManager_getCameraCharacteristics(cameraManager, name, &metadata);
    ACameraMetadata_getAllTags(metadata, &count, &tags);

    entry = {};
    ACameraMetadata_getConstEntry(metadata,
                                  ACAMERA_INFO_SUPPORTED_HARDWARE_LEVEL,
                                  &entry);
    auto hwLevel = static_cast<acamera_metadata_enum_acamera_info_supported_hardware_level>(entry.data.u8[0]);
    LOGI("AndroidCamera::open(): hardwareLevel supported: %d",
         hwLevel);
    // Store the actual sensor array size
    entry = {};
    ACameraMetadata_getConstEntry(metadata,
                                  ACAMERA_SENSOR_INFO_ACTIVE_ARRAY_SIZE,
                                  &entry);
    cameraSensor.width = entry.data.i32[2];
    cameraSensor.height = entry.data.i32[3];
    LOGI("AndroidCamera::open(): Sensor w=%d pix, h=%d pix",
         cameraSensor.width, cameraSensor.height);

    // Store the actual sensor orientation
    entry = {};
    ACameraMetadata_getConstEntry(metadata, ACAMERA_SENSOR_ORIENTATION, &entry);
    cameraSensor.orientation = entry.data.i32[0];
    LOGI("AndroidCamera::open(): Sensor orientation %d degres",
         cameraSensor.orientation);

    // Store the actual sensor lens facing orientation
    entry = {};
    ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &entry);
    cameraSensor.facing = entry.data.u8[0];
#define format_facing(x) ((x == 0) ? "front" : ((x == 1) ? "back" : "external"))
    LOGI("AndroidCamera::open(): Sensor facing %s",
         format_facing(cameraSensor.facing));

    // Store the actual sensor exposure time range
    entry = {};
    ACameraMetadata_getConstEntry(metadata,
                                  ACAMERA_SENSOR_INFO_EXPOSURE_TIME_RANGE,
                                  &entry);
    cameraSensor.minExposure = entry.data.i64[0];
    cameraSensor.maxExposure = entry.data.i64[1];
    LOGI("AndroidCamera::open(): Sensor exposure from %ld to %ld",
         static_cast<long int>(cameraSensor.minExposure),
         static_cast<long int>(cameraSensor.maxExposure));

    // Store the actual sensor sensitivity range
    entry = {};
    ACameraMetadata_getConstEntry(metadata,
                                  ACAMERA_SENSOR_INFO_SENSITIVITY_RANGE,
                                  &entry);
    cameraSensor.minSensitivity = entry.data.i32[0];
    cameraSensor.maxSensitivity = entry.data.i32[1];
    LOGI("AndroidCamera::open(): Sensor sensitivity from %d to %d",
         cameraSensor.minSensitivity, cameraSensor.maxSensitivity);

    // Store the actual sensor frame duration range
    entry = {};
    ACameraMetadata_getConstEntry(metadata,
                                  ACAMERA_SENSOR_INFO_MAX_FRAME_DURATION,
                                  &entry);
    cameraSensor.maxFrameDuration = entry.data.i64[0];

    LOGI("AndroidCamera::open(): Sensor frame duration from %ld ",
         static_cast<long int>(cameraSensor.maxFrameDuration));

    /* Log all metadata for debug 
    for (int tagIdx = 0; tagIdx < count; ++tagIdx)
    {
        entry = {};
        ACameraMetadata_getConstEntry(metadata, tags[tagIdx], &entry);
        LOGI("AndroidCamera::open(): parameter name: %d, value: %d", entry.tag, entry.count);
    }*/

    // Store the camera capabilities as stream outputs
    entry = {};
    ACameraMetadata_getConstEntry(metadata,
                                  ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    for (unsigned int i = 0; i < entry.count; i += 4)
    {
        // We are only interested in output streams, so skip input stream
        bool input = (entry.data.i32[i + 3] != 0);
        if (input)
            continue;

        int32_t format = entry.data.i32[i + 0];
        if ((format != AIMAGE_FORMAT_YUV_420_888) &&
            (format != AIMAGE_FORMAT_JPEG))
            continue;

        int32_t width = entry.data.i32[i + 1];
        int32_t height = entry.data.i32[i + 2];
#define format_string(x) ((x == AIMAGE_FORMAT_JPEG) ? "JPEG" : "YUV420")
        LOGI("AndroidCamera::open(): Mode format=%s, widthu=%d, height=%d",
             format_string(format), width, height);

        captureCapabilities.emplace_back(format, width, height);
    }

    error = ACameraDevice_createCaptureRequest(cameraDevice,
                                               TEMPLATE_STILL_CAPTURE,
                                               &captureRequest);
    if (error != ACAMERA_OK)
    {
        captureRequest = nullptr;
        close();
        return error;
    }

    uint8_t af_mode = ACAMERA_CONTROL_AF_MODE_CONTINUOUS_PICTURE;
    //uint8_t af_mode = ACAMERA_CONTROL_AF_MODE_AUTO;
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_AF_MODE, 1,
                                &af_mode);
    /* FB trials to update parameters */
    //minimum ISO setting depending on sensor when AE is not set
    ACaptureRequest_setEntry_i32(captureRequest, ACAMERA_SENSOR_SENSITIVITY, 1, &cameraSensor.minSensitivity);
    //minimum Time Exposure setting depending on sensor when AE is not set
    if (cameraSensor.maxExposure > static_cast<int64_t>(130000000))
    {
        LOGI("AndroidCamera::open(): exposure range reduced");
        //cameraSensor.maxExposure /= static_cast<int64_t>(5); // divide by 4 for max exposure of N2.2 trial
        cameraSensor.maxExposure = static_cast<int64_t>(125000000);
    }
    ACaptureRequest_setEntry_i64(captureRequest, ACAMERA_SENSOR_EXPOSURE_TIME, 1, &cameraSensor.maxExposure); // standard exposure time lenght

    uint8_t awb_lock = ACAMERA_CONTROL_AWB_LOCK_OFF;  //To be done before AE LOCK OFF
    uint8_t awb_mode = ACAMERA_CONTROL_AWB_MODE_AUTO; //OFF
    uint8_t blacklevel_lock = ACAMERA_BLACK_LEVEL_LOCK_OFF;
    uint8_t scene_mode = ACAMERA_CONTROL_SCENE_MODE_HDR; //scene mode
    uint8_t effect_mode = ACAMERA_CONTROL_EFFECT_MODE_SEPIA;
    uint8_t color_mode = ACAMERA_COLOR_CORRECTION_MODE_FAST; //color correction control overrride by AWB mode AUTO
    uint8_t ae_lock = ACAMERA_CONTROL_AE_LOCK_OFF;           //AutoExposure should be set off in case a camera is not supporting more
    uint8_t ae_mode = ACAMERA_CONTROL_AE_MODE_ON;
    uint8_t control_mode = ACAMERA_CONTROL_MODE_AUTO; // automatic contole of 3A
    //uint8_t recommended_conf = ACAMERA_SCALER_AVAILABLE_RECOMMENDED_STREAM_CONFIGURATIONS_VENDOR_START;
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_AWB_LOCK, 1, &awb_lock);
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_AWB_MODE, 1, &awb_mode);
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_BLACK_LEVEL_LOCK, 1, &blacklevel_lock);
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_SCENE_MODE, 1, &scene_mode);
    //ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_EFFECT_MODE, 1, &effect_mode);
    //ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_COLOR_CORRECTION_MODE, 1, &color_mode);
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_AE_LOCK, 1, &ae_lock);
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_AE_MODE, 1, &ae_mode);
    ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_CONTROL_MODE, 1, &control_mode);
    //ACaptureRequest_setEntry_u8(captureRequest, ACAMERA_SCALER,1, &recommended_conf);
    entry = {};
    ACaptureRequest_getConstEntry(captureRequest,
                                  ACAMERA_CONTROL_AE_MODE,
                                  &entry);
    auto controlAEMode = static_cast<acamera_metadata_enum_acamera_control_ae_mode>(entry.data.u8[0]);
    LOGI("AndroidCamera::open(): CONTROL AE MODE set to  %d", controlAEMode);

    entry = {};
    ACaptureRequest_getConstEntry(captureRequest,
                                  ACAMERA_CONTROL_MODE,
                                  &entry);
    auto controlMode = static_cast<acamera_metadata_enum_android_control_mode_t>(entry.data.u8[0]);
    LOGI("AndroidCamera::open(): CONTROL MODE set to  %d", controlMode);
    return error;
}

int AndroidCamera::setup(const std::string & /*username*/,
                         const std::string & /*password*/) noexcept
{
    /* Nothing to do, we are fine */
    return 0;
}

int AndroidCamera::setup(int &width, int &height) noexcept
{
    auto error = setup(AndroidCamera::Mode(AIMAGE_FORMAT_YUV_420_888,
                                           width, height));
    if (error != ACAMERA_OK)
    {
        return error;
    }

    width = captureMode.width;
    height = captureMode.height;

    return ACAMERA_OK;
}

int AndroidCamera::setup(int &width, int &height,
                         int &screenOrientation) noexcept
{
    auto error = setup(AndroidCamera::Mode(AIMAGE_FORMAT_YUV_420_888,
                                           width, height),
                       screenOrientation);

    if (error != ACAMERA_OK)
    {
        return error;
    }

    return ACAMERA_OK;
}

int AndroidCamera::setup(AndroidCamera::Mode requested) noexcept
{
    return setup(requested, cameraSensor.orientation);
}

int AndroidCamera::setup(AndroidCamera::Mode requested,
                         int screenOrientation) noexcept
{
    ASSERT((cameraDevice != nullptr),
           "AndroidCamera::setup(): Open a camera first!");

    LOGI("AndroidCamera::setup(): requested %dx%d @ %d degrees",
         requested.width, requested.height, screenOrientation);

    // Save the image mode to the requested one
    imageMode = requested;

    // Compute the image rotation with regards to the sensor and screen
    // orientations, as well as the lens facing orientation
    int lensOrientation = (((screenOrientation % 180) == 90) &&
                           (cameraSensor.facing == 0))
                              ? 180
                              : 0;
    auto rotation = (-screenOrientation + lensOrientation +
                     cameraSensor.orientation + 720) %
                    360;
    imageRotation = (static_cast<int32_t>(round((rotation / 90.0f)))) % 4;

    // If the requested image rotation requires an odd number of quarter
    // rotations, then swap the width and heights to compare to the sensor
    // capabilities
    if ((imageRotation % 2) == 1)
        requested.swapWidthHeight();

    // Define a broken mode having huge width and height
    Mode mode = {0, 0x7FFFFFFF, 0x7FFFFFFF};

    for (const auto capability : captureCapabilities)
    {
        if (requested == capability)
        {
            mode = capability;
            break;
        }
        else if ((requested.format == capability.format) &&
                 (requested.width <= capability.width) &&
                 (requested.height <= capability.height) &&
                 (capability.width <= mode.width) &&
                 (capability.height <= mode.height))
        {
            mode = capability;
        }
    }

    // Cannot find the requested mode of capture
    if (mode.format == 0)
        return ACAMERA_ERROR_INVALID_PARAMETER;

    // If already configured, we are done!
    if (mode == captureMode)
        return ACAMERA_OK;

    // Otherwise unsetup the previous mode cleanly before setting up a new one
    unsetup();

    // Set the image reader
    auto media_error = AImageReader_new(mode.width, mode.height, mode.format, 1,
                                        &imageReader);

    if (media_error != AMEDIA_OK)
    {
        unsetup();
        if (media_error == AMEDIA_ERROR_INVALID_PARAMETER)
            return ACAMERA_ERROR_INVALID_PARAMETER;
        else
            return ACAMERA_ERROR_UNKNOWN;
    }
    captureMode = mode;

    AImageReader_setImageListener(imageReader, &imageListener);
    AImageReader_getWindow(imageReader, &imageWindow);
    ACameraOutputTarget_create(imageWindow, &imageTarget);
    ACaptureRequest_addTarget(captureRequest, imageTarget);
    ACaptureSessionOutputContainer_create(&imageOutputs);
    ACaptureSessionOutput_create(imageWindow, &imageOutput);
    ACaptureSessionOutputContainer_add(imageOutputs, imageOutput);

    auto error = ACameraDevice_createCaptureSession(cameraDevice, imageOutputs,
                                                    &captureStateCallbacks,
                                                    &captureSession);
    if (error != ACAMERA_OK)
    {
        unsetup();
        return error;
    }

    return ACAMERA_OK;
}

void AndroidCamera::unsetup() noexcept
{
    // Exit if nothing has been setup
    if (captureMode.format == 0)
        return;

    LOGD("AndroidCamera::unsetup(): Closing capture session");
    if (captureSession != nullptr)
    {
        ACameraCaptureSession_close(captureSession);
        captureSession = nullptr;
    }

    LOGD("AndroidCamera::unsetup(): Removing capture session output container");
    ACaptureSessionOutputContainer_remove(imageOutputs, imageOutput);
    LOGD("AndroidCamera::unsetup(): Freeing capture session output");
    ACaptureSessionOutput_free(imageOutput);
    imageOutput = nullptr;

    LOGD("AndroidCamera::unsetup(): Freeing capture session output container");
    ACaptureSessionOutputContainer_free(imageOutputs);
    imageOutputs = nullptr;

    LOGD("AndroidCamera::unsetup(): Removing capture request target");
    ACaptureRequest_removeTarget(captureRequest, imageTarget);
    LOGD("AndroidCamera::unsetup(): Freeing CameraOutputTarget");
    ACameraOutputTarget_free(imageTarget);
    imageTarget = nullptr;

    LOGD("AndroidCamera::unsetup(): "
         "Deleting image reader (and auto-releasing image window)");
    AImageReader_delete(imageReader);
    imageReader = nullptr;
    imageWindow = nullptr;

    captureMode = Mode(0, 0, 0);
    imageMode = Mode(0, 0, 0);
    LOGD("AndroidCamera::unsetup(): Unsetup done");
}

int AndroidCamera::read(cv::Mat &image) noexcept
{
    // Use a default session if none exists so far...
    if (captureSession == nullptr)
    {
        LOGD("AndroidCamera::read(): "
             "No camera setup made, switching to a default mode");
        auto error = setup(Mode(640, 480, 0));
        if (error != ACAMERA_OK)
            return error;
    }

    // Launch a capture
    std::promise<int> promise;
    std::future<int> completion = promise.get_future();

    imageReaderCallbacks.push({&promise, &image});
    ACameraCaptureSession_capture(captureSession, nullptr, 1,
                                  &captureRequest, nullptr);

    auto error = completion.get();
    if (error != ACAMERA_OK)
    {
        LOGE("AndroidCamera::read(): Error %d", error);
        return error;
    }

    // Get what should be the right capture mode
    auto reqMode = imageMode.swapWidthHeightIf((imageRotation % 2) == 1);
    if (!(reqMode == captureMode))
    {
        cv::Mat orig(std::move(image));
        cv::Rect from;
        if ((captureMode.width * reqMode.height) >
            (reqMode.width * captureMode.height))
        {
            auto w = (captureMode.height * reqMode.width) / reqMode.height;
            from = cv::Rect((captureMode.width - w) / 2, 0, w, captureMode.height);
        }
        else
        {
            auto h = (captureMode.width * reqMode.height) / reqMode.width;
            from = cv::Rect(0, (captureMode.height - h) / 2, captureMode.width, h);
        }
        cv::resize(cv::Mat(orig, from), image,
                   cv::Size(reqMode.width, reqMode.height), 0, 0,
                   cv::INTER_AREA);
    }

    if (imageRotation != 0)
    {
        cv::RotateFlags rotation;
        switch (imageRotation)
        {
        case 1:
            rotation = cv::ROTATE_90_CLOCKWISE;
            break;
        case 2:
            rotation = cv::ROTATE_180;
            break;
        case 3:
            rotation = cv::ROTATE_90_COUNTERCLOCKWISE;
            break;
        default:
            ASSERT(false, "AndroidCamera::read(): "
                          "Invalid imageRotation requested: %d",
                   imageRotation);
            return ACAMERA_ERROR_INVALID_PARAMETER;
        }
        cv::Mat orig(std::move(image));
        cv::rotate(orig, image, rotation);
    }

    return ACAMERA_OK;
}

const AndroidCamera::Sensor &AndroidCamera::sensor() const noexcept
{
    return cameraSensor;
}

const AndroidCamera::Mode &AndroidCamera::mode() const noexcept
{
    return imageMode;
}

const std::vector<AndroidCamera::Mode> &
AndroidCamera::capabilities() const noexcept
{
    return captureCapabilities;
}

int AndroidCamera::close() noexcept
{
    int error = ACAMERA_OK;

    LOGD("AndroidCamera::close(): Closing camera '%s'", cameraName.c_str());

    unsetup();

    if (captureRequest != nullptr)
    {
        LOGD("AndroidCamera::close(): Freeing capture request");
        ACaptureRequest_free(captureRequest);
        captureRequest = nullptr;
    }

    if (cameraDevice != nullptr)
    {
        LOGD("AndroidCamera::close(): Closing camera device");
        error = ACameraDevice_close(cameraDevice);
        cameraDevice = nullptr;
    }

    cameraName.clear();
    cameraSensor = {};
    captureCapabilities.clear();

    return error;
}

} // namespace IO
} // namespace Util
