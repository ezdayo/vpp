#
# This is a portable configuration for running code lint
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#

# Using CPP linter for tracking code issues
find_program(CPPLINT_EXECUTABLE
	     NAMES cpplint
    	     DOC "Path to cpplint executable")
mark_as_advanced(CPPLINT_EXECUTABLE)

if(CPPLINT_EXECUTABLE)
	# Run the code linter along with the currently compiled file
	set(CMAKE_CXX_CPPLINT "${CPPLINT_EXECUTABLE};--filter=-build/include_order;--verbose=0")

	# Add a specific lint target
	add_custom_target(lint
		COMMAND -${CPPLINT_EXECUTABLE} --verbose=3 --filter=-build/include_order --counting=detailed --repository=${CMAKE_CURRENT_SOURCE_DIR}/src --recursive ${CMAKE_CURRENT_SOURCE_DIR}/src
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Linting the whole code source tree."
        	VERBATIM)
else(NOT CPPLINT_EXECUTABLE)
	message(STATUS "Cannot find any cpplint binary: disabling CPP lints.")
endif()

