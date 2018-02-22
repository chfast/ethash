cmake_minimum_required(VERSION 3.5)

include(cmake/cable/bootstrap.cmake)

include(CableToolchains)
include(HunterGate)

cable_configure_toolchain(DEFAULT cxx11)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.20.10.tar.gz"
    SHA1 "5e7b471fb4704f23638588e91f53f2394f1babb4"
)

project(ethash)

add_subdirectory(lib/ethash)
