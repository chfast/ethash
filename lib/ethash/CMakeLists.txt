set(include_dir ${PROJECT_SOURCE_DIR}/include)

add_library(
    ethash
    endianness.hpp
    ${include_dir}/ethash/ethash.hpp ethash-internal.hpp ethash.cpp
    keccak.hpp keccak.c
    params.hpp
    utils.hpp utils.cpp
)

target_include_directories(ethash PUBLIC ${include_dir})
target_link_libraries(ethash PRIVATE ethash-buildinfo)

get_filename_component(ETHASH_PRIVATE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)
set(ETHASH_PRIVATE_INCLUDE_DIR ${ETHASH_PRIVATE_INCLUDE_DIR} PARENT_SCOPE)