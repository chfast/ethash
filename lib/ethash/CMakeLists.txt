set(include_dir ${PROJECT_SOURCE_DIR}/include)

add_library(
    ethash
    ${include_dir}/ethash/ethash.hpp ethash.cpp
    keccak.hpp keccak.cpp
    keccak-tiny.c
    params.hpp
    types.hpp
    utils.hpp utils.cpp
)

target_include_directories(ethash PUBLIC ${include_dir})

get_filename_component(ETHASH_PRIVATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
set(ETHASH_PRIVATE_INCLUDE_DIR ${ETHASH_PRIVATE_INCLUDE_DIR} PARENT_SCOPE)