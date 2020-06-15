#
# This is a portable configuration for running code check
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#

# Using CPP checker for tracking code issues
find_program(CPPCHECK_EXECUTABLE
	     NAMES cppcheck
    	     DOC "Path to cppcheck executable")
mark_as_advanced(CPPCHECK_EXECUTABLE)

if(CPPCHECK_EXECUTABLE)
	# Run the code checker along with the currently compiled file
	set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXECUTABLE};--enable=warning,portability,information,missingInclude;--suppress=missingIncludeSystem;--inconclusive;--force;--inline-suppr")

	# Add a specific check target
	add_custom_target(check
		COMMAND ${CPPCHECK_EXECUTABLE} --project=compile_commands.json --std=c++${CMAKE_CXX_STANDARD} --enable=all --inconclusive --force --inline-suppr --verbose
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        	COMMENT "Checking the whole code source tree."
        	VERBATIM)
else(NOT CPPCHECK_EXECUTABLE)
	message(STATUS "Cannot find any cppcheck binary: disabling CPP checks.")
endif()


