/**
 *
 * @file      vpp/util/io/realsense.cpp
 *
 * @brief     This is the Intel Realsense Input class definition
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

#include <bitset>
#include <cstring>
#include <librealsense2/rs.hpp>
#include <unordered_map>

#include "vpp/error.hpp"
#include "vpp/log.hpp"
#include "vpp/scene.hpp"
#include "vpp/util/io/realsense.hpp"

namespace Util {
namespace IO {

class Realsense::Core : public Util::OCV::ProjectionDelegate {
public:
    Core(const char *device) noexcept;
    ~Core() noexcept;

    Core(const Core &other) = delete;
    Core(Core &&other) = delete;
    Core &operator=(const Core &other) = delete;
    Core &operator=(Core &&other) = delete;

    std::vector<std::string> modes(rs2_stream id) noexcept;
    int use(rs2_stream id) noexcept; 
    int setup(rs2_stream id, int &width, int &height) noexcept;
    int get(rs2_stream id, cv::Mat &image) noexcept;
    int release(rs2_stream id) noexcept;

    cv::Point project(const cv::Point3f &p) const noexcept override;
    cv::Point3f deproject(const cv::Point &p,
                          uint16_t z) const noexcept override;

private:
    void sync() noexcept;
    std::string                                 serial;
    std::bitset<RS2_STREAM_COUNT>               used;
    std::bitset<RS2_STREAM_COUNT>               configured;
    rs2::config                                 cfg;
    rs2::pipeline                               pipe;
    rs2::pipeline_profile                       running;
    rs2::align                                  align_to_color;
    rs2::frameset                               data;
    std::unordered_map<int, rs2::frame>         frame;
    std::unordered_map<int, unsigned long long> last_frame_id;
    std::unordered_map<int, unsigned long long> used_frame_id;
    rs2_intrinsics                              intrinsics;
    float                                       depth_scale;
};

/*
 * Static utility functions
 */

/* Getting a realsense device (core) from an id */
static rs2::context context;
static rs2::device_list rs_devices(context.query_devices());
static std::unordered_map<std::string, Realsense::Core> cores;

static void discover() {
    rs_devices = context.query_devices();
}

static rs2::device device_at(int id) {
    ASSERT((id >= 0) && (id < static_cast<int>(rs_devices.size())),
           "Realsense: no device for index %d", id);

    return rs_devices[id];
}

static const char *device_id_at(int id) {
    if ((id >= 0) && (id < static_cast<int>(rs_devices.size()))) {
        return device_at(id).get_info(RS2_CAMERA_INFO_SERIAL_NUMBER);
    }
    return nullptr;
}

static Realsense::Core *core_for(int id) {
    // Use a copy of the device_id
    auto rs_did = device_id_at(id);
    if (rs_did == nullptr) {
        return nullptr;
    }

    std::string copy(rs_did);
    const char *did = copy.c_str();
    
    auto found = cores.find(did);
    if (found != cores.end()) {
        return &found->second;
    }
    
    cores.emplace(did, did);
    
    return &cores.find(did)->second;
}

/* Getting a stream id from a protocol string */
#define RS2_STREAM_DEPTH_STR    "rs/depth"
#define RS2_STREAM_COLOR_STR    "rs/color"
static int stream_for(const std::string &protocol) {
    
    static std::unordered_map<std::string, int> stream_of_string({
            { RS2_STREAM_COLOR_STR, RS2_STREAM_COLOR },
            { RS2_STREAM_DEPTH_STR, RS2_STREAM_DEPTH } });

    auto found = stream_of_string.find(protocol);
    if (found != stream_of_string.end()) {
        return found->second;
    }

    return -1;
}

/*
 * Realsense core interface methods
 */

Realsense::Core::Core(const char *device) noexcept
    : Util::OCV::ProjectionDelegate(), serial(device), used(), configured(),
      cfg(), pipe(), running(), align_to_color(RS2_STREAM_COLOR), data(),
      frame(), last_frame_id(), used_frame_id(), intrinsics(), depth_scale(0) {
    LOGD("Realsense::Core::Core(%s)", serial.c_str());
    cfg.enable_device(serial.c_str());
}

Realsense::Core::~Core() noexcept {
    LOGD("Realsense::Core::~Core(%s)", serial.c_str());
}

std::vector<std::string> Realsense::Core::modes(rs2_stream id) noexcept {
    std::set<std::pair<int, int>> unique_modes; // ordered set
    std::vector<std::string>      valid_modes;

    /* Go through all the devices to find out where our device is... */
    for (auto&& dev : rs_devices) {
        if (serial == dev.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)) {
            for (auto&& s : dev.query_sensors()) {
                for (auto&& p : s.get_stream_profiles()) {
                    auto v = p.as<rs2::video_stream_profile>();
                    if ( (v) && (v.stream_type() == id) && 
                         ( (v.format() == RS2_FORMAT_Z16) ||
                           (v.format() == RS2_FORMAT_BGR8) ) ) {
                        unique_modes.emplace(std::pair<int, int>(v.width(), 
                                                                 v.height()));
                    }
                }
            }
            break;
        }
    }

    for (auto &m : unique_modes) {
        valid_modes.emplace_back(std::to_string(m.first) + "x" + 
                                 std::to_string(m.second));
        LOGD("Realsense::Core::modes(%s@%d): %dx%d", serial.c_str(), id, 
             m.first, m.second);
    }

    return valid_modes;
}

int Realsense::Core::use(rs2_stream id) noexcept {
    if (used[id]) {
        return Error::INVALID_REQUEST;
    }

    used[id] = true;
    return Error::NONE;
}

int Realsense::Core::setup(rs2_stream id, int &width, int &height) noexcept {
    if (!used[id]) {
        return Error::INVALID_REQUEST;
    }

    if (running) { // If pipe was already running then stop to reconfigure
        pipe.wait_for_frames(); 
        pipe.stop();
    }

    cfg.enable_stream(id, width, height, (id == RS2_STREAM_COLOR) ? 
                                          RS2_FORMAT_BGR8 : RS2_FORMAT_Z16);

    running = pipe.start(cfg);

    auto geom = running.get_stream(id).as<rs2::video_stream_profile>();
    width  = geom.width();
    height = geom.height();

    configured[id] = true;

    last_frame_id.emplace(id, 0);
    used_frame_id.emplace(id, 0);

    if (id == RS2_STREAM_DEPTH) {
        depth_scale = running.get_device().first<rs2::depth_sensor>()
                      .get_depth_scale();
        intrinsics = geom.get_intrinsics();
        LOGD("Realsense::Core::setup(%s@%d): depth_scale = %f", 
              serial.c_str(), id, depth_scale);
    }
    
    LOGD("Realsense::Core::setup(%s@%d): %dx%d", serial.c_str(), id, 
          width, height);
        
    return Error::NONE;
}

int Realsense::Core::get(rs2_stream id, cv::Mat &image) noexcept {
    if (!configured[id]) {
        return Error::INVALID_REQUEST;
    }

    /* Wait for new frames if and only if we already processed this set of
     * frames */
    if (last_frame_id[id] == used_frame_id[id]) {
        auto has_depth = configured[RS2_STREAM_DEPTH];
        auto has_color = configured[RS2_STREAM_COLOR];

        while ( (has_depth && (used_frame_id[RS2_STREAM_DEPTH] == 
                               last_frame_id[RS2_STREAM_DEPTH]) ) || 
                (has_color && (used_frame_id[RS2_STREAM_COLOR] == 
                               last_frame_id[RS2_STREAM_COLOR]) ) ) {

            // Wait for the next set of frames
            data = pipe.wait_for_frames();

            // Make sure the frames are spatially aligned
            if (has_depth && has_color) {
                data = align_to_color.process(data);
            }

            // Update the color frame
            if (has_color) {
                frame[RS2_STREAM_COLOR] = data.get_color_frame();
                last_frame_id[RS2_STREAM_COLOR] =
                                    frame[RS2_STREAM_COLOR].get_frame_number();
            }

            // Update the depth frame
            if (has_depth) {
                frame[RS2_STREAM_DEPTH]         = data.get_depth_frame();
                last_frame_id[RS2_STREAM_DEPTH] = 
                                    frame[RS2_STREAM_DEPTH].get_frame_number();
            }

        }
    }

    const int w = frame[id].as<rs2::video_frame>().get_width();
    const int h = frame[id].as<rs2::video_frame>().get_height();
    image = std::move(cv::Mat(cv::Size(w, h), 
                              (id == RS2_STREAM_COLOR) ? CV_8UC3 : CV_16UC1,
                              (void*)frame[id].get_data(), cv::Mat::AUTO_STEP));
    used_frame_id[id] = last_frame_id[id];

    return Error::NONE; 
}

int Realsense::Core::release(rs2_stream id) noexcept {
    LOGD("Realsense::Core::release(%s@%d)", serial.c_str(), id);
    if (!used[id]) {
        return Error::INVALID_REQUEST;
    }

    switch (id) {
        case RS2_STREAM_DEPTH:
        case RS2_STREAM_COLOR:
            cfg.disable_stream(id);
            break;

        default:
            break;
    }

    if (running) { // If pipe was already running then stop to reconfigure
        pipe.wait_for_frames(); 
        pipe.stop();
    }

    configured[id] = false;
    used[id]       = false;
    
    frame.erase(id);
    last_frame_id.erase(id);
    used_frame_id.erase(id);

    if (used.any()) {
        running = pipe.start(cfg);
    } else {
        running = rs2::pipeline_profile();
        cores.erase(serial);
    }
     
    return Error::NONE;
}

cv::Point Realsense::Core::project(const cv::Point3f &p) const noexcept {
    /* If no projection is in use then do none! */
    if (depth_scale == 0) {
        return cv::Point(p.x, p.y);
    }

    /* Otherwise, do the computation */
    float x = p.x / p.z;
    float y = p.y / p.z;

    if(intrinsics.model == RS2_DISTORTION_MODIFIED_BROWN_CONRADY)
    {
        float r2  = x*x + y*y;
        float f = 1 + (intrinsics.coeffs[0] + 
                  (intrinsics.coeffs[1] + intrinsics.coeffs[4]*r2)*r2)*r2;
        x *= f;
        y *= f;
        float dx = x + 2*intrinsics.coeffs[2]*x*y + 
                   intrinsics.coeffs[3]*(r2 + 2*x*x);
        float dy = y + 2*intrinsics.coeffs[3]*x*y + 
                   intrinsics.coeffs[2]*(r2 + 2*y*y);
        x = dx;
        y = dy;
    }

    if (intrinsics.model == RS2_DISTORTION_FTHETA)
    {
        float r = sqrt(x*x + y*y);
        float rd = (1.0f / intrinsics.coeffs[0] * 
                    atan(2 * r* tan(intrinsics.coeffs[0] / 2.0f)));
        x *= rd / r;
        y *= rd / r;
    }

    return cv::Point(x * intrinsics.fx + intrinsics.ppx, 
                     y * intrinsics.fy + intrinsics.ppy);
}

cv::Point3f Realsense::Core::deproject(const cv::Point &p, 
                                       uint16_t z) const noexcept {
    /* If no projection is in use then do none! */
    if (depth_scale == 0) {
        return cv::Point3f(p.x, p.y, 0);
    }

    /* Otherwise, do the computation */
    ASSERT(intrinsics.model != RS2_DISTORTION_MODIFIED_BROWN_CONRADY,
           "Realsense::Core::deproject() : "
           "Cannot deproject from a forward-distorted image");
    ASSERT(intrinsics.model != RS2_DISTORTION_FTHETA,
           "Realsense::Core::deproject() : "
           "Cannot deproject to an FTheta image");

    float x  = (p.x - intrinsics.ppx) / intrinsics.fx;
    float y  = (p.y - intrinsics.ppy) / intrinsics.fy;
    float uz = depth_scale*z;
    if(intrinsics.model == RS2_DISTORTION_INVERSE_BROWN_CONRADY)
    {
        float r2  = x*x + y*y;
        float f = 1 + (intrinsics.coeffs[0] + (intrinsics.coeffs[1] + 
                       intrinsics.coeffs[4]*r2)*r2)*r2;
        float ux = x*f + 2*intrinsics.coeffs[2]*x*y +
                   intrinsics.coeffs[3]*(r2 + 2*x*x);
        float uy = y*f + 2*intrinsics.coeffs[3]*x*y +
                   intrinsics.coeffs[2]*(r2 + 2*y*y);
        x = ux;
        y = uy;
    }

    return cv::Point3f(uz*x, uz*y, uz);
}

/*
 * Realsense interface methods
 */

Realsense::Realsense() noexcept : 
    Input({RS2_STREAM_COLOR_STR, RS2_STREAM_DEPTH_STR}), 
    stream(-1), core(nullptr) {}

Realsense::~Realsense() noexcept {
    close();
}

std::vector<std::string> Realsense::sources() const noexcept {
    /* Update the sources */
    discover();
    
    std::vector<std::string> devices;
    for (int i=0; i < static_cast<int>(rs_devices.size()); ++i) {
        devices.emplace_back(std::to_string(i));
    }

    return std::move(devices);
}

std::vector<std::string> Realsense::modes() noexcept {

    std::vector<std::string> devices;
    if (core != nullptr) {
        return core->modes(static_cast<rs2_stream>(stream));
    }

    /* Cannot find modes of an unopened device! */
    return std::vector<std::string>();
}

int Realsense::open(const std::string &protocol, int id) noexcept {
    ASSERT(supports(protocol),
           "Realsense::open(): unsupported protocol %s",
           protocol.c_str());

    auto dev = core_for(id);
    if (dev == nullptr) {
        return Error::INVALID_VALUE;
    }

    if (supports(protocol)) {
        auto stm = stream_for(protocol);
        auto error = dev->use(static_cast<rs2_stream>(stm));
        if (error != Error::NONE) {
            return error;
        }

        stream = stm;
        core   = dev;

        return Error::NONE;
    }

    /* Wrong protocol requested */
    return Error::INVALID_VALUE;
}

int Realsense::open(const std::string &protocol,
                    const std::string &source) noexcept {
    int src;
    std::istringstream data(source);

    data >> src;
    auto status = data.rdstate();
    if (status != (std::ios::goodbit|std::ios::eofbit)) {
        return Error::INVALID_VALUE;
    }
    return open(protocol, src);
}

int Realsense::setup(const std::string & /*username*/,
                     const std::string & /*password*/) noexcept {
    /* Nothing to do, we are fine */
    return Error::NONE;
}

int Realsense::setup(int &width, int &height, int &rotation) noexcept {
    if (core != nullptr) {
        rotation = 0;
        return core->setup(static_cast<rs2_stream>(stream), width, height);
    }

    /* Cannot setup a non-opened device */
    return Error::INVALID_REQUEST;
}

int Realsense::read(cv::Mat &image) noexcept {
    if (core != nullptr) {
        return core->get(static_cast<rs2_stream>(stream), image);
    }

    /* Cannot close a non-opened device */
    return Error::INVALID_REQUEST;
}

int Realsense::close() noexcept {
    if (core != nullptr) {
        auto error = core->release(static_cast<rs2_stream>(stream));
        stream = -1;
        core   = nullptr;
        return error;
    }

    /* Cannot close a non-opened device */
    return Error::INVALID_REQUEST;
}

Util::OCV::ProjectionDelegate *Realsense::projection() const noexcept {
    return core;
}

}  // namespace IO
}  // namespace Util
