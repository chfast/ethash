# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

add_executable(
    ethash-test
    helpers.hpp
    test_bit_manipulation.cpp
    test_cases.hpp
    test_ethash.cpp
    test_keccak.cpp
    test_kiss.cpp
    test_managed.cpp
    test_primes.cpp
    test_progpow.cpp
)

target_link_libraries(ethash-test PRIVATE ethash GTest::gtest GTest::main)
target_include_directories(ethash-test PRIVATE ${ETHASH_PRIVATE_INCLUDE_DIR})
set_target_properties(ethash-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ..)

if(ETHASH_PGO)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-generate=${profdata_dir}")
    get_target_property(sources ethash-test SOURCES)
    add_executable(ethash-test-profgen ${sources})
    get_target_property(include_directories ethash-test INCLUDE_DIRECTORIES)
    target_link_libraries(ethash-test-profgen PRIVATE ethash-profgen GTest::gtest GTest::main)
    target_include_directories(ethash-test-profgen PUBLIC ${include_directories})
    set_target_properties(ethash-test-profgen PROPERTIES RUNTIME_OUTPUT_DIRECTORY ..)

    add_custom_target(profdata-generate ethash-test-profgen --gtest_filter=ethash.verify_hash:progpow.hash_and_verify)

    add_custom_target(profdata-merge llvm-profdata-7 merge -output=${profdata_dir}/ethash.profdata ${profdata_dir})
    add_dependencies(profdata-merge profdata-generate)
endif()
