#
# Helper function for pre-configuring specific types of files
#
# This file is part of the Customisation framework (see link).
#
# Author:   Olivier Stoltz-Douchet <ezdayo@gmail.com>
#
# Copyright (c) 2019-2020 Olivier Stoltz-Douchet
# License:  http://opensource.org/licenses/MIT MIT
# Link:     https://github.com/ezdayo/customisation
#

# Helper function for configuring any kind of file
function(configure_this_file file_name dst_dir)
    set(src_file ${PROJECT_SOURCE_DIR}/tpl/${file_name}.in)
    set(dst_file ${dst_dir}/${file_name})
    
    configure_file(${src_file} ${dst_file} @ONLY)
    unset(src_file CACHE)
    unset(dst_file CACHE)
endfunction()

# Helper function for configuring a header file
function (configure_header_file file_name)
    configure_this_file(${file_name} ${PROJECT_SOURCE_DIR}/src)
endfunction()

# Helper function for configuring a doxygen file
function (configure_doxy_file file_name)
    configure_this_file(${file_name} ${PROJECT_SOURCE_DIR}/doc)
endfunction()

