// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "progpow-internal.hpp"

#include "bit_manipulation.h"
#include "endianness.hpp"
#include "kiss99.h"

#include <ethash/keccak.h>

namespace progpow
{
hash256 keccak_progpow_256(
    const hash256& header_hash, uint64_t nonce, const uint32_t extra[4]) noexcept
{
    static constexpr size_t num_words = sizeof(header_hash.hwords) / sizeof(header_hash.hwords[0]);

    uint32_t state[25] = {};

    size_t i;
    for (i = 0; i < num_words; ++i)
        state[i] = le::uint32(header_hash.hwords[i]);

    state[i++] = static_cast<uint32_t>(nonce);
    state[i++] = static_cast<uint32_t>(nonce >> 32);

    if (extra)
    {
        for (int j = 0; j < 4; ++j)
            state[i++] = extra[j];
    }

    ethash_keccakf800(state);

    hash256 output;
    for (i = 0; i < num_words; ++i)
        output.hwords[i] = state[i];
    return output;
}

uint64_t keccak_progpow_64(
    const hash256& header_hash, uint64_t nonce, const uint32_t* extra) noexcept
{
    const hash256 h = keccak_progpow_256(header_hash, nonce, extra);
    return (uint64_t(h.hwords[0]) << 32) | h.hwords[1];
}

mix_state init(uint64_t seed) noexcept
{
    const uint32_t seed_lo = static_cast<uint32_t>(seed);
    const uint32_t seed_hi = static_cast<uint32_t>(seed >> 32);

    const uint32_t z = fnv1a(0x811c9dc5, seed_lo);
    const uint32_t w = fnv1a(z, seed_hi);
    const uint32_t jsr = fnv1a(w, seed_lo);
    const uint32_t jcong = fnv1a(jsr, seed_hi);
    mix_state state{{z, w, jsr, jcong}, {{}}};

    // Create a random sequence of mix destinations for merge()
    // guaranteeing every location is touched once.
    // Uses Fisher–Yates shuffle.
    for (uint32_t i = 0; i < num_regs; ++i)
        state.index_sequence[i] = i;

    for (uint32_t i = num_regs; i > 1; --i)
    {
        uint32_t j = kiss99_generate(&state.rng_state) % i;
        std::swap(state.index_sequence[i - 1], state.index_sequence[j]);
    }
    return state;
}

ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW
uint32_t random_math(uint32_t a, uint32_t b, uint32_t selector) noexcept
{
    switch (selector % 11)
    {
    default:
    case 0:
        return a + b;
    case 1:
        return a * b;
    case 2:
        return mul_hi32(a, b);
    case 3:
        return std::min(a, b);
    case 4:
        return rotl32(a, b);
    case 5:
        return rotr32(a, b);
    case 6:
        return a & b;
    case 7:
        return a | b;
    case 8:
        return a ^ b;
    case 9:
        return clz32(a) + clz32(b);
    case 10:
        return popcount32(a) + popcount32(b);
    }
}

}  // namespace progpow
