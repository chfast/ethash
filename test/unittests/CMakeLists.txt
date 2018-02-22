# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

add_executable(
    ethash-test
    test_ethash.cpp
    test_keccak.cpp
    test_utils.cpp
)

target_link_libraries(ethash-test PRIVATE ethash GTest::gtest GTest::main)
target_include_directories(ethash-test PRIVATE ${ETHASH_PRIVATE_INCLUDE_DIR})
message("XX: ${ETHASH_PRIVATE_INCLUDE_DIR}")