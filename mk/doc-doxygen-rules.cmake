#
# This is a portable configuration for running doxygen on the whole code tree
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#

# Using doxygen for generating the documentation
find_package(Doxygen)

if(DOXYGEN_FOUND)
    # Create the Doxyfile
    include(mk/configure-file-rules.cmake)
    configure_doxy_file(Doxyfile)

    # Add a specific doc target
    add_custom_target(doc
	    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/doc/Doxyfile
	    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc
            COMMENT "Generating API documentation with Doxygen"
	    VERBATIM)
 
else(NOT DOXYGEN_FOUND)
    message(STATUS "Cannot find any doxygen binary: disabling doc generation.")
endif()

