# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

add_library(test-compile-c90 STATIC compilation_test.c)
target_link_libraries(test-compile-c90 PRIVATE ethash)
set_target_properties(test-compile-c90 PROPERTIES C_STANDARD 90 C_EXTENSIONS OFF)

add_library(test-compile-c99 STATIC compilation_test.c)
target_link_libraries(test-compile-c99 PRIVATE ethash)
set_target_properties(test-compile-c99 PROPERTIES C_STANDARD 99 C_EXTENSIONS OFF)

add_library(test-compile-c11 STATIC compilation_test.c)
target_link_libraries(test-compile-c11 PRIVATE ethash)
set_target_properties(test-compile-c11 PROPERTIES C_STANDARD 11 C_EXTENSIONS OFF)
