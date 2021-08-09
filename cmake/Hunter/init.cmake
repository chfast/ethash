# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2021 Pawel Bylica.
# Licensed under the Apache License, Version 2.0.

include(HunterGate)

if(NOT WIN32)
    # Outside of Windows build only Release packages.
    set(HUNTER_CONFIGURATION_TYPES Release
        CACHE STRING "Build type of the Hunter packages")
endif()

HunterGate(
    URL https://github.com/cpp-pm/hunter/archive/v0.23.239.tar.gz
    SHA1 135567a8493ab3499187bce1f2a8df9b449febf3
    LOCAL
)
