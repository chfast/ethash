# Cable: CMake Bootstrap Library.
# Copyright 2018 Pawel Bylica.
# Licensed under the Apache License, Version 2.0. See the LICENSE file.

set(CMAKE_SYSTEM_PROCESSOR mips64)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER mips64-linux-gnuabi64-gcc)
set(CMAKE_CXX_COMPILER mips64-linux-gnuabi64-g++)

set(CMAKE_FIND_ROOT_PATH /usr/mips64-linux-gnuabi64)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CROSSCOMPILING_EMULATOR qemu-mips64-static;-L;${CMAKE_FIND_ROOT_PATH})
