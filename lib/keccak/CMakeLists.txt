# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2019 Pawel Bylica.
# Licensed under the Apache License, Version 2.0.

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

enable_language(ASM)
set(CMAKE_ASM_FLAGS "-Wno-unused-command-line-argument")

add_library(
    keccak
    ${include_dir}/ethash/keccak.h
    ${include_dir}/ethash/keccak.hpp
    keccak.c
    keccakf800.c
    keccakf1600.c
    xkcp.c
    KeccakP-1600-AVX2.s
    KeccakP-1600-x86-64-gas.s
    openssl_keccak1600.c
)

target_include_directories(keccak PUBLIC $<BUILD_INTERFACE:${include_dir}>$<INSTALL_INTERFACE:include>)

install(
    TARGETS keccak
    EXPORT ethashTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
