# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

cmake_minimum_required(VERSION 3.5)

include(cmake/cable/bootstrap.cmake)

include(CableCompilerSettings)
include(CableToolchains)
include(HunterGate)

cable_configure_toolchain(DEFAULT cxx11)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.20.10.tar.gz"
    SHA1 "5e7b471fb4704f23638588e91f53f2394f1babb4"
)

project(ethash)

cable_configure_compiler()

add_subdirectory(lib/ethash)

option(ETHASH_BUILD_TESTS "Build unit tests" ON)
if(ETHASH_BUILD_TESTS)
    add_subdirectory(test/unittests)
endif()
