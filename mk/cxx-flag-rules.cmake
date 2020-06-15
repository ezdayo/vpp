#
# Helper function for adding a CXX compiler flag if it is supported
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#

# Use the check_cxx_compiler_flag_if_supported function
include(CheckCXXCompilerFlag)

# Helper function for adding a CXX compiler flag if it is supported
function(enable_cxx_compiler_flag_if_supported flag)
    string(FIND "${CMAKE_CXX_FLAGS}" "${flag}" flag_already_set)
    if(flag_already_set EQUAL -1)
        check_cxx_compiler_flag("${flag}" flag_supported)
        if(flag_supported)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
        else()
	    message(STATUS "Warning: Discarding unsupported flag ${flag}")
        endif()
        unset(flag_supported CACHE)
    endif()
endfunction()

