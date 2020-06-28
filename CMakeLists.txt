#
# These are the CMakeLists for the Visual Pipeline Processor (VPP) library
#
# This script is a CMake script aimed at generating the build environment for
# the VPP library
#
# This file is part of the VPP framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/vpp
#

# VPP is a C++ project
project(VPP LANGUAGES CXX)

# The project version number.
set(VERSION_MAJOR   0   CACHE STRING "Project major version number.")
set(VERSION_MINOR   9   CACHE STRING "Project minor version number.")
set(VERSION_PATCH   0   CACHE STRING "Project patch version number.")
set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR} rev. ${VERSION_PATCH}")
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH VERSION_FULL)

# CMake version 3.1+ is required (to enable C++11 options)
# CMake version 3.6+ is required (to enable clang-tidy commands)
# CMake version 3.10+ is required (to enable cppcheck commands)
cmake_minimum_required(VERSION 3.1)

# Runtime library dependencies
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/mk)
find_package(PkgConfig)
if(ANDROID)
pkg_check_modules(BriJNI REQUIRED brijni)
pkg_check_modules(NNPACK REQUIRED nnpack)
pkg_check_modules(OpenCV REQUIRED libopencv_video libopencv_videoio
	          libopencv_dnn libopencv_imgcodecs libopencv_imgproc
		  libopencv_objdetect libopencv_features2d libopencv_core)
pkg_check_modules(FFMPEG REQUIRED libavformat libavcodec libswscale
	          libswresample libavutil)
pkg_check_modules(LEPTONICA REQUIRED lept)
pkg_check_modules(PNG REQUIRED libpng)
pkg_check_modules(SSH2 REQUIRED libssh2)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED libcrypto libssl)
else()
find_package(OpenCV COMPONENTS core imgproc REQUIRED)
find_package(OpenCV COMPONENTS dnn freetype imgcodecs highgui objdetect
       	                       video videoio xfeatures2d)
endif()

pkg_check_modules(CUSTOMISATION customisation REQUIRED)
pkg_check_modules(DARKNET darknet)
pkg_check_modules(RS realsense2-gl)
pkg_check_modules(TESSERACT tesseract)
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

# C++ compiler options for all build configurations 
set (CMAKE_CXX_STANDARD 11)
include(mk/cxx-flag-rules.cmake)
enable_cxx_compiler_flag_if_supported(-Wall)
enable_cxx_compiler_flag_if_supported(-Wextra)
enable_cxx_compiler_flag_if_supported(-pedantic)
enable_cxx_compiler_flag_if_supported(-Wno-noexcept-type)
enable_cxx_compiler_flag_if_supported(-fPIC)
enable_cxx_compiler_flag_if_supported("${EXTRA_CXX_FLAGS}")

#
# Necessary for link-time optimisation LTO
#
if(NOT ANDROID)
set(CMAKE_AR  "gcc-ar")
set(CMAKE_RANLIB  "gcc-ranlib")
endif()

# Add code quality related targets and rules
include(mk/cxx-code-quality-rules.cmake)

#
# Binary targets
#

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)
endif()

# Configure header files
if(DARKNET_FOUND)
	set(VPP_HAS_DARKNET_SUPPORT TRUE)
endif()

if(OPENCV_DNN_FOUND)
	set(VPP_HAS_OPENCV_DNN_SUPPORT TRUE)
endif()

if(OPENCV_FREETYPE_FOUND)
	set(VPP_HAS_EXTERNAL_FONT_SUPPORT TRUE)
endif()

if(OPENCV_HIGHGUI_FOUND)
	set(VPP_HAS_OPENCV_GUI_SUPPORT TRUE)
endif()

if(OPENCV_IMGCODECS_FOUND)
	set(VPP_HAS_IMAGE_CODEC_SUPPORT TRUE)
endif()

if(OPENCV_OBJDETECT_FOUND)
	set(VPP_HAS_SIMILARITY_CLUSTERING_SUPPORT TRUE)
endif()

if(OPENCV_VIDEO_FOUND)
	set(VPP_HAS_TRACKING_SUPPORT TRUE)
endif()

if(OPENCV_VIDEOIO_FOUND)
	set(VPP_HAS_OPENCV_VIDEO_IO_SUPPORT TRUE)
endif()

if(OPENCV_XFEATURES2D_FOUND OR OPENCV_FEATURES2D_FOUND)
	set(VPP_HAS_FEATURE_DETECTION_SUPPORT TRUE)
endif()

if(RS_FOUND)
	set(VPP_HAS_REALSENSE_CAPTURE_SUPPORT TRUE)
endif()

if(TESSERACT_FOUND)
	set(VPP_HAS_TESSERACT_SUPPORT TRUE)
endif()

configure_file (${PROJECT_SOURCE_DIR}/tpl/config.hpp.in
	        ${PROJECT_SOURCE_DIR}/inc/vpp/config.hpp @ONLY)

# VPP library configuration
include_directories(${PROJECT_SOURCE_DIR}/inc ${PROJECT_SOURCE_DIR}/src
	            ${BriJNI_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}
	 	    ${DARKNET_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS}
		    ${TESSERACT_INCLUDE_DIRS} ${LEPTONICA_INCLUDE_DIRS}
		    ${PNG_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS}
		    ${SSH2_INCLUDE_DIRS} ${CUSTOMISATION_INCLUDE_DIRS}
		    ${Readline_INCLUDE_DIRS})

add_definitions(-DLOGTAG="VPP")
set(LIB_FILES ${PROJECT_SOURCE_DIR}/inc/vpp/config.hpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/dnn/dataset.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/dnn/engine.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/dnn/setup.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/blur.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/bridge.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/capture.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/clustering.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/ocr/edging.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/tracker/history.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/tracker/none.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/engine/overlay.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/image.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/log.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/logo.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/pipeline.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/scene.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage/blur.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage/clustering.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage/dnn.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage/input.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage/ocr/edging.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage/ocr/reader.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/stage/overlay.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/task.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/task/blur.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/task/clustering.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/task/edging.cpp
	       #${PROJECT_SOURCE_DIR}/src/vpp/tracker.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/ui/overlay.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/util/io/image.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/util/io/input.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/util/ocv/functions.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/util/ocv/overlay.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/util/task.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/util/utf8.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/view.cpp
	       ${PROJECT_SOURCE_DIR}/src/vpp/zone.cpp)

# Add specific files depending on available libraries
if(VPP_HAS_DARKNET_SUPPORT)
	set(LIB_FILES ${LIB_FILES}
	       	      ${PROJECT_SOURCE_DIR}/src/vpp/engine/detector/darknet.cpp)
endif()

if(VPP_HAS_OPENCV_DNN_SUPPORT)
	set(LIB_FILES ${LIB_FILES}
	       	      ${PROJECT_SOURCE_DIR}/src/vpp/dnn/ocv.cpp
	       	      ${PROJECT_SOURCE_DIR}/src/vpp/engine/classifier/ocv.cpp
	       	      ${PROJECT_SOURCE_DIR}/src/vpp/engine/detector/ocv.cpp) 
endif()

if(VPP_HAS_OPENCV_VIDEO_IO_SUPPORT)
	set(LIB_FILES ${LIB_FILES}
	       	      ${PROJECT_SOURCE_DIR}/src/vpp/util/ocv/capture.cpp)
endif()

if(VPP_HAS_FEATURE_DETECTION_SUPPORT)
	set(LIB_FILES ${LIB_FILES}
	              ${PROJECT_SOURCE_DIR}/src/vpp/engine/ocr/mser.cpp
	              ${PROJECT_SOURCE_DIR}/src/vpp/stage/ocr/mser.cpp
	              ${PROJECT_SOURCE_DIR}/src/vpp/task/mser.cpp)
endif()

if(VPP_HAS_REALSENSE_CAPTURE_SUPPORT)
	set(LIB_FILES ${LIB_FILES}
	              ${PROJECT_SOURCE_DIR}/src/vpp/util/io/realsense.cpp)
endif()

if(VPP_HAS_TESSERACT_SUPPORT)
	set(LIB_FILES ${LIB_FILES}
	              ${PROJECT_SOURCE_DIR}/src/vpp/engine/ocr/tesseract.cpp)
endif()

if(VPP_HAS_TRACKING_SUPPORT)
	set(LIB_FILES ${LIB_FILES}
	              ${PROJECT_SOURCE_DIR}/src/vpp/kernel/histogram.cpp
	              ${PROJECT_SOURCE_DIR}/src/vpp/kernel/kalman.cpp
		      ${PROJECT_SOURCE_DIR}/src/vpp/engine/tracker/kalman.cpp
		      ${PROJECT_SOURCE_DIR}/src/vpp/task/kernel/histogram.cpp
		      ${PROJECT_SOURCE_DIR}/src/vpp/task/kernel/kalman.cpp)
endif()
if(ANDROID)
	set(LIB_FILES ${LIB_FILES}
	    ${PROJECT_SOURCE_DIR}/src/vpp/util/io/android_camera.cpp
	    # Next source files are to remove !!!
	    ${PROJECT_SOURCE_DIR}/src/vpp/deprecated/angine.cpp
	    ${PROJECT_SOURCE_DIR}/src/vpp/dnn/deprecated/dnn.cpp
	    ${PROJECT_SOURCE_DIR}/src/vpp/dnn/deprecated/darknet_dnn.cpp 
	    ${PROJECT_SOURCE_DIR}/src/vpp/dnn/deprecated/opencv_dnn.cpp
	    ${PROJECT_SOURCE_DIR}/src/vpp/ocr/deprecated/ocr.cpp
	    ${PROJECT_SOURCE_DIR}/src/vpp/ocr/deprecated/tesseract_ocr.cpp
	    ${PROJECT_SOURCE_DIR}/src/vpp/util/io/deprecated/source.cpp)
endif()

# Add platform specific files
if(ANDROID)
	set(JNI_FILES ${LIB_FILES}
	    ${PROJECT_SOURCE_DIR}/src/dscribe/pipeline.cpp
	    # Next source files are to update !!!
	    ${PROJECT_SOURCE_DIR}/src/dscribe/deprecated/hmi/jni.cpp
	    ${PROJECT_SOURCE_DIR}/src/dscribe/deprecated/deescribe.cpp
	    ${PROJECT_SOURCE_DIR}/src/dscribe/deprecated/description.cpp
	    ${PROJECT_SOURCE_DIR}/src/dscribe/deprecated/dscribe.jni.cpp)
else()
	set(EXE_FILES ${PROJECT_SOURCE_DIR}/src/dscribe/pipeline.cpp
	              ${PROJECT_SOURCE_DIR}/src/dscribe/cli.cpp)
endif()

if(ANDROID)
add_library(vpp SHARED ${JNI_FILES})
target_link_libraries(vpp ${OpenCV_STATIC_LDFLAGS} ${FFMPEG_LDFLAGS}
		      ${CURL_STATIC_LDFLAGS} ${SSH2_STATIC_LDFLAGS}
		      ${OPENSSL_STATIC_LDFLAGS} ${DARKNET_LDFLAGS}
		      ${NNPACK_STATIC_LDFLAGS} ${TESSERACT_STATIC_LDFLAGS}
		      ${LEPTONICA_STATIC_LDFLAGS} ${PNG_STATIC_LDFLAGS}
		      ${BriJNI_STATIC_LDFLAGS} ${CUSTOMISATION_STATIC_LDFLAGS}
		      ${RS_LDFLAGS} Threads::Threads 
		      -ljnigraphics -landroid -Wl,-Bsymbolic)

set_target_properties(vpp PROPERTIES PUBLIC_HEADER
	 	      "inc/hmi/dscribe.jni.hpp;")
install(TARGETS vpp
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include)
else()

# Building the VPP shared (dynamic) library
add_library(vpp SHARED ${LIB_FILES})
target_link_libraries(vpp ${OpenCV_LIBRARIES} ${CUSTOMISATION_STATIC_LDFLAGS}
		      ${TESSERACT_LDFLAGS} ${RS_LDFLAGS} 
		      ${DARKNET_LDFLAGS} Threads::Threads)

# Installing the VPP shared library
install(TARGETS vpp 
	LIBRARY DESTINATION lib)

# Installing the VPP includes
install(DIRECTORY ${PROJECT_SOURCE_DIR}/inc/vpp/
	DESTINATION include/vpp
	FILES_MATCHING PATTERN "*.hpp"
	PATTERN "*/deprecated/*" EXCLUDE
	PATTERN "*/deprecated" EXCLUDE)

# Building the d-scribe executable
add_executable(d-scribe EXCLUDE_FROM_ALL ${EXE_FILES})
target_link_libraries(d-scribe vpp)

endif()

#
# Non binary targets
#

# Add documentation generation targets and rules
include(mk/doc-doxygen-rules.cmake)

