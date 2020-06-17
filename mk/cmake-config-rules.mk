#
# This is the Makefile for defining multiple builds of different targets
# Default UNIX commands (if not defined yet)
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#

CD?=cd
CMAKE?=cmake
CP?=cp -a
MAKE?=make
MKDIR?=mkdir -p
TOUCH?=touch

# Build and output directories (if not defined yet)
BLD_DIR?=bld
OUT_DIR?=out

# Build type ("debug" is default if none is specified)
BUILD?=debug
ifeq ($(BUILD), debug)					# Debug build
BLD:=debug
BLD_TYPE:=Debug
else ifeq ($(BUILD), info)				# Release w/ debug info
BLD:=info
BLD_TYPE:=RelWithDebInfo
else ifeq ($(BUILD), size)				# Size optimisation
BLD:=size
BLD_TYPE:=MinSizeRel
else ifeq ($(BUILD), $(filter $(BUILD), speed release))	# Speed optimisation
BLD:=speed
BLD_TYPE:=Release
else							# Error !
$(error Unknown build version "$(BUILD)", use one of debug|info|size|speed.)
endif

# Default values for the code checker, code linter and code tidier options
CMAKE_OPTIONS:=	-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DCMAKE_BUILD_TYPE=$(BLD_TYPE)
MAKE_OPTIONS:=

# Target OS ("linux" is the default if none is specified)
OS?=linux
ifeq ($(OS), linux)					# Linux build
ABI?=amd64

else ifeq ($(OS), android)				# Android build
ABI?=armeabi-v7a
CMAKE_OPTIONS+= -DANDROID_ABI="$(ABI)"

ifdef ANDROID_NDK
CMAKE_OPTIONS+= -DANDROID_NDK=$(ANDROID_NDK)
MAKE_OPTIONS+= ANDROID_NDK=$(ANDROID_NDK)
endif

ANDROID_NATIVE_API_LEVEL?=24
CMAKE_OPTIONS+= -DANDROID_NATIVE_API_LEVEL=$(ANDROID_NATIVE_API_LEVEL)
MAKE_OPTIONS+= ANDROID_NATIVE_API_LEVEL=$(ANDROID_NATIVE_API_LEVEL)

ANDROID_NDK_HOST_X64?=1
CMAKE_OPTIONS+= -DANDROID_NDK_HOST_X64=$(ANDROID_NDK_HOST_X64)
MAKE_OPTIONS+= ANDROID_NDK_HOST_X64=$(ANDROID_NDK_HOST_X64)

else ifeq ($(OS), ios)					# iOS build
ABI?=arm64-v8a

else							# Error !
$(error Unknown target "$(OS)", use one of linux|android|ios.)
endif

# Evaluate specific build variables
empty:=
space:=$(empty) $(empty)

ifdef CMAKE_TOOLCHAIN
CMAKE_OPTIONS+= -DCMAKE_TOOLCHAIN_FILE=$(CMAKE_TOOLCHAIN)
MAKE_OPTIONS+= CMAKE_TOOLCHAIN=$(CMAKE_TOOLCHAIN)
endif

ifdef EXTRA_C_FLAGS
CMAKE_OPTIONS+= -DEXTRA_C_FLAGS="$(EXTRA_C_FLAGS)"
MAKE_OPTIONS+= EXTRA_C_FLAGS="$(EXTRA_C_FLAGS)"
endif

ifdef EXTRA_CXX_FLAGS
CMAKE_OPTIONS+= -DEXTRA_CXX_FLAGS="$(EXTRA_CXX_FLAGS)"
MAKE_OPTIONS+= EXTRA_CXX_FLAGS="$(EXTRA_CXX_FLAGS)"
endif

# Specific location for the OpenCV CMake configuration file
ifdef OPENCV_DIR
CMAKE_OPTIONS+= -DOpenCV_DIR=$(OPENCV_DIR)
MAKE_OPTIONS+= OPENCV_DIR=$(OPENCV_DIR)
endif

# CODE quality enablers
ifdef CODE_CHECK
CMAKE_OPTIONS+=	-DCODE_CHECK=$(CODE_CHECK)
MAKE_OPTIONS+=	CODE_CHECK=$(CODE_CHECK)
endif

ifdef CODE_LINT
CMAKE_OPTIONS+=	-DCODE_LINT=$(CODE_LINT)
MAKE_OPTIONS+=	CODE_LINT=$(CODE_LINT)
endif

ifdef CODE_TIDY
CMAKE_OPTIONS+=	-DCODE_TIDY=$(CODE_TIDY)
MAKE_OPTIONS+=	CODE_TIDY=$(CODE_TIDY)
endif

ifdef CODE_FIX
CMAKE_OPTIONS+=	-DCODE_FIX=$(CODE_FIX)
MAKE_OPTIONS+=	CODE_FIX=$(CODE_FIX)
endif

# Target directories aliases (for faster typing)
BLD_VER_DIR:=$(BLD_DIR)/$(OS)/$(ABI)/$(BLD)
OUT_VER_DIR:=$(OUT_DIR)/$(OS)/$(ABI)/$(BLD)

# Add target aliases in multiple build versions
%-debug:
	$(MAKE) $(MAKE_OPTIONS) BUILD=debug $*
%-info:
	$(MAKE) $(MAKE_OPTIONS) BUILD=info $*
%-release:
	$(MAKE) $(MAKE_OPTIONS) BUILD=release $*
%-speed:
	$(MAKE) $(MAKE_OPTIONS) BUILD=speed $*
%-size:
	$(MAKE) $(MAKE_OPTIONS) BUILD=size $*

%-android:
	$(MAKE) $(MAKE_OPTIONS) OS=android $*
%-ios:
	$(MAKE) $(MAKE_OPTIONS) OS=ios $*
%-linux:
	$(MAKE) $(MAKE_OPTIONS) OS=linux $*

%-armeabi-v7a %-armv7:
	$(MAKE) $(MAKE_OPTIONS) ABI=armeabi-v7a $*
%-arm64-v8a %-arm64:
	$(MAKE) $(MAKE_OPTIONS) ABI=arm64-v8a $*
%-i386:
	$(MAKE) $(MAKE_OPTIONS) ABI=i386 $*
%-x64 %-amd64:
	$(MAKE) $(MAKE_OPTIONS) ABI=amd64 $*

# Add target aliases to enable lint, check, fix, tidy configurations
config-enable-code-check:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_CHECK=ON config
config-disable-code-check:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_CHECK=OFF config
config-enable-code-lint:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_LINT=ON config
config-disable-code-lint:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_LINT=OFF config
config-enable-code-tidy:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_TIDY=ON config
config-disable-code-tidy:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_TIDY=OFF config
config-enable-fix-code-tidy:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_FIX=ON config
config-disable-fix-code-tidy:
	$(MAKE) $(MAKE_OPTIONS) OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) \
		CODE_FIX=OFF config

config-enable-all-code-analysis:
	$(MAKE) CODE_CHECK=ON CODE_LINT=ON CODE_TIDY=ON \
		OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) config
config-disable-all-code-analysis:
	$(MAKE) CODE_CHECK=OFF CODE_LINT=OFF CODE_TIDY=OFF CODE_FIX=OFF \
		OS=$(OS) ABI=$(ABI) BUILD=$(BUILD) config

# Create the target output directory and copy the build target
$(OUT_VER_DIR)/%:	$(BLD_VER_DIR)/%
	$(MKDIR) $(OUT_VER_DIR)
	$(CP)	$< $@

# Create the target build directory, its CMake Makefiles and the compile
# commands JSON database
$(BLD_VER_DIR)/Makefile:	Makefile CMakeLists.txt $(wildcard mk/* tpl/*)

config $(BLD_VER_DIR)/Makefile:
	$(MKDIR) -p $(BLD_VER_DIR)
	$(CD) $(BLD_VER_DIR) && \
	$(CMAKE) $(CMAKE_OPTIONS) \
		 -DEXTRA_SRC_FILES="$(subst $(space),;,$(EXTRA_SRC_FILES))" \
 		 ../../../..

# Build the target from the generated Makefile
# (strip the "lib" prefix and the ".xxx" extension for the target)
$(BLD_VER_DIR)/%:	$(BLD_VER_DIR)/Makefile for_cmake
	$(CD) $(BLD_VER_DIR) && \
		$(MAKE) VERBOSE=1 $(patsubst lib%,%,$(basename $*))

# Checking, documenting or linting the whole source code tree
code-format-check code-format-fix check doc lint: \
					$(BLD_VER_DIR)/Makefile for_cmake
	$(CD) $(BLD_VER_DIR) && $(MAKE) $@
 
# Meant to always call the CMake-created Makefile in the sub-directory
for_cmake:

# Keep precious build files not to reinvent the wheel all the time!
.PRECIOUS:	$(BLD_VER_DIR)/%

# Non file targets
.PHONY: %-debug %-info %-release %-size %-speed %-android %-ios %-linux \
	%-amd64 %-arm64 %-arm64-v8a %-armeabi-v7a %-armv7 %-i386 %-x64 \ 
	code-format-check code-format-fix check config doc for_cmake lint \
	config-enable-code-check config-disable-code-check \
	config-enable-code-lint config-disable-code-lint \
	config-enable-code-tidy config-disable-code-tidy \
	config-enable-fix-code-tidy config-disable-fix-code-tidy \
	config-enable-all-code-analysis config-disable-all-code-analysis
