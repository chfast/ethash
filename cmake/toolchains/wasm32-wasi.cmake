if(NOT DEFINED WASI_SDK_PREFIX)
    # Set default value of WASI_SDK_PREFIX
    set(WASI_SDK_PREFIX /opt/wasi-sdk)
endif()
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES WASI_SDK_PREFIX)

if(NOT WASI_SDK_PREFIX STREQUAL /opt/wasi-sdk)
    # Set --sysroot. Not necessary if WASI_SDK_PATH is /opt/wasi-sdk.
    set(CMAKE_SYSROOT ${WASI_SDK_PREFIX}/share/wasi-sysroot)
endif()

include(${WASI_SDK_PREFIX}/share/cmake/wasi-sdk.cmake)
