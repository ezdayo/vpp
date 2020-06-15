#
# This is a portable configuration for running code check, lint and tidy on
# C++ code
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#


# Options for using a code checker, code linter and code tidier
option(CODE_CHECK "Run the C++ code checker" OFF)
option(CODE_LINT "Run the C++ code linter" OFF)
option(CODE_TIDY "Run the C++ code tidier" OFF)
option(CODE_FIX "Run the C++ code tidy fixer" OFF)

# Options for the C++ compiler to have a more robust code
include(mk/cxx-flag-rules.cmake)
enable_cxx_compiler_flag_if_supported("-Wall")
enable_cxx_compiler_flag_if_supported("-Wextra")
enable_cxx_compiler_flag_if_supported("-pedantic")
if(CODE_CHECK OR CODE_LINT)
enable_cxx_compiler_flag_if_supported("-Wshadow")
enable_cxx_compiler_flag_if_supported("-Wnon-virtual-dtor")
enable_cxx_compiler_flag_if_supported("-Wold-style-cast")
enable_cxx_compiler_flag_if_supported("-Wcast-align")
enable_cxx_compiler_flag_if_supported("-Wunused")
enable_cxx_compiler_flag_if_supported("-Woverloaded-virtual")
enable_cxx_compiler_flag_if_supported("-Wconversion")
enable_cxx_compiler_flag_if_supported("-Wsign-conversion")
enable_cxx_compiler_flag_if_supported("-Wduplicated-cond")
enable_cxx_compiler_flag_if_supported("-Wduplicated-branches")
enable_cxx_compiler_flag_if_supported("-Wlogical-op")
enable_cxx_compiler_flag_if_supported("-Wnull-dereference")
enable_cxx_compiler_flag_if_supported("-Wuseless-cast")
enable_cxx_compiler_flag_if_supported("-Wdouble-promotion")
enable_cxx_compiler_flag_if_supported("-Wformat=2")
endif(CODE_CHECK OR CODE_LINT)
if(CODE_TIDY)
enable_cxx_compiler_flag_if_supported("-Wmisleading-indentation")
endif(CODE_TIDY)

# Using CPP checker for tracking code issues
if(CODE_CHECK)
include(mk/cxx-code-check-rules.cmake)
endif(CODE_CHECK)

# Using CPP linter for tracking code style issues
if(CODE_LINT)
include(mk/cxx-code-lint-rules.cmake)
endif(CODE_LINT)

# Using Clang-Tidy for tracking code and style issues
if(CODE_TIDY)
include(mk/cxx-code-tidy-rules.cmake)
if(CODE_FIX AND CLANG_TIDY_EXECUTABLE)
set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY};-fix;-fix-errors")
endif(CODE_FIX AND CLANG_TIDY_EXECUTABLE)
endif(CODE_TIDY)

# Using CLang-Forat for reformatting the code
include(mk/cxx-code-format-rules.cmake)

