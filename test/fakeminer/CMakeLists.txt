# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

add_executable(fakeminer fakeminer.cpp)
target_link_libraries(fakeminer PRIVATE ethash)
