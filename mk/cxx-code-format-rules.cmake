#
# This is a portable configuration for running code formatter
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#

# Using CLang format for re-formatting the source code
find_program(CLANG_FORMAT_EXECUTABLE
	     NAMES clang-format
    	     DOC "Path to the clang-format executable")
     mark_as_advanced(CLANG_FORMAT_EXECUTABLE)

# Get the list of files to re-format
file(GLOB_RECURSE CLANG_REFORMAT_FILES
     "${PROJECT_SOURCE_DIR}/src/*.h"
     "${PROJECT_SOURCE_DIR}/src/*.hpp"
     "${PROJECT_SOURCE_DIR}/src/*.cpp"
     "${PROJECT_SOURCE_DIR}/src/*.cc"
     "${PROJECT_SOURCE_DIR}/src/*.c"
     "${PROJECT_SOURCE_DIR}/src/*.java"
     "${PROJECT_SOURCE_DIR}/src/*.mm"
     "${PROJECT_SOURCE_DIR}/inc/*.h"
     "${PROJECT_SOURCE_DIR}/inc/*.hpp"
     "${PROJECT_SOURCE_DIR}/inc/*.cpp"
     "${PROJECT_SOURCE_DIR}/inc/*.cc"
     "${PROJECT_SOURCE_DIR}/inc/*.c"
     "${PROJECT_SOURCE_DIR}/inc/*.java"
     "${PROJECT_SOURCE_DIR}/inc/*.mm")

if(CLANG_FORMAT_EXECUTABLE)
	add_custom_target(code-format-fix
		COMMAND ${CLANG_FORMAT_EXECUTABLE} -sort-includes=false -verbose -i ${CLANG_REFORMAT_FILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Fixing the format of the whole code source tree."
        	VERBATIM)
	add_custom_target(code-format-check
		COMMAND ${CLANG_FORMAT_EXECUTABLE} -sort-includes=false -verbose -output-replacements-xml ${CLANG_REFORMAT_FILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Checking the right format to the whole code source tree."
        	VERBATIM)
else(NOT CLANG_FORMAT_EXECUTABLE)
	message(STATUS "Cannot find any clang-format-diff binary: disabling format checking.")
endif()

