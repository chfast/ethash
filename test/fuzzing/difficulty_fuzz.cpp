// intx: extended precision integer library.
// Copyright 2021 Pawel Bylica.
// Licensed under the Apache License, Version 2.0.

#include "../experimental/difficulty.h"
#include "../unittests/helpers.hpp"
#include "../lib/ethash/ethash-internal.hpp"
#include <cstring>
#include <iostream>

constexpr size_t input_size = sizeof(ethash_hash256);

extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t max_size);

extern "C" size_t LLVMFuzzerCustomMutator(
    uint8_t* data, size_t size, size_t max_size, unsigned int /*seed*/) noexcept
{
    if (max_size >= 2 * input_size)
        size = 2 * input_size;
    else if (max_size >= input_size)
        size = input_size;
    return LLVMFuzzerMutate(data, size, max_size);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t data_size) noexcept
{
    using namespace ethash;

    if (data_size < input_size)
        return 0;

    ethash_hash256 difficulty;
    std::memcpy(difficulty.bytes, data, sizeof(difficulty));

    ethash_hash256 boundary = ethash_difficulty_to_boundary(&difficulty);

    auto hash = boundary;
    if (data_size >= 2 * input_size)
        std::memcpy(hash.bytes, data + input_size, sizeof(hash));

    const auto check_difficulty = check_against_difficulty(hash, difficulty);
    const auto check_boundary = less_equal(hash, boundary);

    if (check_boundary != check_difficulty)
    {
        std::cerr << "FAILED:\n" << to_hex(difficulty) << "\n" << to_hex(hash) << "\n";
        __builtin_trap();
    }

    return 0;
}
