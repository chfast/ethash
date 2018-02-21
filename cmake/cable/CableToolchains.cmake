# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

set(cable_toolchain_dir ${CMAKE_CURRENT_LIST_DIR}/toolchains)

function(cable_configure_toolchain)
    cmake_parse_arguments("" "" "DEFAULT" ""  ${ARGN})

    set(toolchain default)
    if(_DEFAULT)
        set(toolchain ${_DEFAULT})
    endif()

    set(toolchain_file ${cable_toolchain_dir}/${toolchain}.cmake)

    set(CMAKE_TOOLCHAIN_FILE ${toolchain_file} CACHE FILEPATH "CMake toolchain file")
endfunction()
