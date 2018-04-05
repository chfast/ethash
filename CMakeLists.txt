# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

cmake_minimum_required(VERSION 3.5)

include(cmake/cable/bootstrap.cmake)

include(CableBuildType)
include(CableCompilerSettings)
include(CableToolchains)
include(HunterGate)

include(defaults/HunterCacheServers)

cable_configure_toolchain(DEFAULT cxx11)

set(HUNTER_CONFIGURATION_TYPES Release
    CACHE STRING "Build type of the Hunter packages")

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.20.35.tar.gz"
    SHA1 "6e3cb4c333b76803a83c56fcbca3b0bfd9f96f27"
)

project(ethash)
set(PROJECT_VERSION 0.1.0.dev0)

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)

cable_configure_compiler()
if(CABLE_COMPILER_GNULIKE)
    # Do not allow lossy type conversions.
    add_compile_options(-Wconversion -Wsign-conversion)

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Og")

    if(NOT CMAKE_CROSSCOMPILING)
        # Tune for currently most common CPUs (flag not supported in mips64 cross-compiler).
        add_compile_options(-mtune=generic)
    endif()
endif()

add_subdirectory(lib/ethash)

option(ETHASH_BUILD_TESTS "Build unit tests" ON)
if(ETHASH_BUILD_TESTS)
    add_subdirectory(test)
endif()
