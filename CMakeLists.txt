cmake_minimum_required(VERSION 3.5)

include(cmake/cable/bootstrap.cmake)

include(CableToolchains)

cable_configure_toolchain(DEFAULT cxx11)

project(ethash)

add_subdirectory(lib/ethash)
