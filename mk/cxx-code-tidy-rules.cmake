#
# This is a portable configuration for running code tidy
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#


# Using CPP tidier for tracking code issues
find_program(CLANG_TIDY_EXECUTABLE
	     NAMES clang-tidy
    	     DOC "Path to clang_tidy executable")
mark_as_advanced(CLANG_TIDY_EXECUTABLE)

if(CLANG_TIDY_EXECUTABLE)
	# Run the code tidyer along with the currently compiled file
	set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-p=${CMAKE_CURRENT_BINARY_DIR};-checks=*,-clang-diagnostic-unused-command-line-argument,-fuchsia-overloaded-operator,-fuchsia-default-arguments;-extra-arg=-Wno-unknown-warning-option;-format-style=llvm")
else(NOT CLANG_TIDY_EXECUTABLE)
	message(STATUS "Cannot find any clang_tidy binary: disabling code tidy.")
endif()
