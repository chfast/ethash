# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2019 Pawel Bylica.
# SPDX-License-Identifier: Apache-2.0

include(GNUInstallDirs)

add_library(keccak)
add_library(ethash::keccak ALIAS keccak)
target_compile_features(keccak PUBLIC c_std_99)
target_include_directories(keccak PUBLIC $<BUILD_INTERFACE:${include_dir}>$<INSTALL_INTERFACE:include>)
target_sources(keccak PRIVATE
    ${include_dir}/ethash/keccak.h
    ${include_dir}/ethash/keccak.hpp
    keccak.c
    keccakf800.c
)

install(
    TARGETS keccak
    EXPORT ethashTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
