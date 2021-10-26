// intx: extended precision integer library.
// Copyright 2021 Pawel Bylica.
// Licensed under the Apache License, Version 2.0.

#include "../experimental/difficulty.h"
#include "../unittests/helpers.hpp"
#include <ethash/hash_types.h>
#include <cstring>
#include <iostream>

constexpr size_t input_size = sizeof(ethash_hash256);

NO_SANITIZE("unsigned-integer-overflow")
NO_SANITIZE("unsigned-shift-base")
static inline uint64_t umul_add_to(uint64_t* p, uint64_t x, uint64_t y, uint64_t k) noexcept
{
    // const auto r = intx::umul(x, y) + a + b;
    // return {r[0], r[1]};

    // Portable full unsigned multiplication 64 x 64 -> 128.
    uint64_t xl = x & 0xffffffff;
    uint64_t xh = x >> 32;
    uint64_t yl = y & 0xffffffff;
    uint64_t yh = y >> 32;

    uint64_t t0 = xl * yl;
    uint64_t t1 = xh * yl;
    uint64_t t2 = xl * yh;
    uint64_t t3 = xh * yh;

    uint64_t u1 = t1 + (t0 >> 32);
    uint64_t u2 = t2 + (u1 & 0xffffffff);

    uint64_t plo = (u2 << 32) | (t0 & 0xffffffff);
    uint64_t phi = t3 + (u2 >> 32) + (u1 >> 32);

    *p += plo;
    phi += (*p < plo);

    *p += k;
    phi += (*p < k);
    return phi;
}

NO_SANITIZE("unsigned-integer-overflow")
bool validate(const ethash_hash256& difficulty, const ethash_hash256& boundary) noexcept
{
    uint64_t d[4];
    for (int i = 0; i < 4; ++i)
        d[i] = __builtin_bswap64(difficulty.word64s[4 - 1 - i]);

    uint64_t b[4];
    for (int i = 0; i < 4; ++i)
        b[i] = __builtin_bswap64(boundary.word64s[4 - 1 - i]);

    uint64_t p[8]{};
    for (size_t j = 0; j < 4; ++j)
    {
        uint64_t k = 0;
        for (size_t i = 0; i < 4; ++i)
            k = umul_add_to(&p[i + j], b[i], d[j], k);
        p[j + 4] = k;
    }

    const auto top_zero = (p[7] | p[6] | p[5]) == 0;
    const auto overflow = !top_zero | (p[4] != 0);
    const auto low_zero = (p[3] | p[2] | p[1] | p[0]) == 0;
    const auto high_one = top_zero & (p[4] == 1);

    return !overflow | (low_zero & high_one);
}


extern "C" size_t LLVMFuzzerMutate(uint8_t* data, size_t size, size_t max_size);

extern "C" size_t LLVMFuzzerCustomMutator(
    uint8_t* data, size_t size, size_t max_size, unsigned int /*seed*/) noexcept
{
    if (max_size >= input_size)
        size = input_size;
    return LLVMFuzzerMutate(data, size, max_size);
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t data_size) noexcept
{
    if (data_size < input_size)
        return 0;


    ethash_hash256 difficulty;
    std::memcpy(difficulty.bytes, data, sizeof(difficulty));

    ethash_hash256 boundary = ethash_difficulty_to_boundary(&difficulty);

    bool valid = validate(difficulty, boundary);

    if (!valid)
    {
        std::cerr << "FAILED:\n" << to_hex(difficulty) << "\n" << to_hex(boundary) << "\n";
        __builtin_trap();
    }

    return 0;
}
