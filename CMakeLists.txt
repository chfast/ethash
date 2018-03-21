# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

cmake_minimum_required(VERSION 3.5)

include(cmake/cable/bootstrap.cmake)

include(CableBuildInfo)
include(CableBuildType)
include(CableCompilerSettings)
include(CableToolchains)
include(HunterGate)

cable_configure_toolchain(DEFAULT cxx11)

set(HUNTER_CONFIGURATION_TYPES Release
    CACHE STRING "Build type of the Hunter packages")

set(HUNTER_CACHE_SERVERS https://github.com/ethereum/hunter-cache
    CACHE STRING "Hunter cache servers")

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.20.10.tar.gz"
    SHA1 "5e7b471fb4704f23638588e91f53f2394f1babb4"
)

project(ethash)
set(PROJECT_VERSION 0.1.0.dev0)

cable_set_build_type(DEFAULT RelWithDebInfo CONFIGURATION_TYPES Debug Release RelWithDebInfo)

cable_configure_compiler()
if(CABLE_COMPILER_GNULIKE)
    # Do not allow lossy type conversions.
    add_compile_options(-Wconversion)
endif()

cable_add_buildinfo_library(PREFIX ethash)

add_subdirectory(lib/ethash)

option(ETHASH_BUILD_TESTS "Build unit tests" ON)
if(ETHASH_BUILD_TESTS)
    add_subdirectory(test)
endif()
