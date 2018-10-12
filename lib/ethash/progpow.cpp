// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "progpow-internal.hpp"

#include "bit_manipulation.h"
#include "endianness.hpp"
#include "ethash-internal.hpp"
#include "kiss99.hpp"

#include <ethash/keccak.h>

namespace progpow
{
hash256 keccak_progpow_256(
    const hash256& header_hash, uint64_t nonce, const hash256& mix_hash) noexcept
{
    static constexpr size_t num_words = sizeof(header_hash.hwords) / sizeof(header_hash.hwords[0]);

    uint32_t state[25] = {};

    size_t i;
    for (i = 0; i < num_words; ++i)
        state[i] = le::uint32(header_hash.hwords[i]);

    state[i++] = static_cast<uint32_t>(nonce);
    state[i++] = static_cast<uint32_t>(nonce >> 32);

    for (uint32_t mix_word : mix_hash.hwords)
        state[i++] = mix_word;

    ethash_keccakf800(state);

    hash256 output;
    for (i = 0; i < num_words; ++i)
        output.hwords[i] = le::uint32(state[i]);
    return output;
}

uint64_t keccak_progpow_64(
    const hash256& header_hash, uint64_t nonce, const hash256& mix_hash) noexcept
{
    const hash256 h = keccak_progpow_256(header_hash, nonce, mix_hash);
    // FIXME: BE mess.
    return (uint64_t(le::uint32(h.hwords[0])) << 32) | le::uint32(h.hwords[1]);
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
        uint32_t j = state.rng() % i;
        std::swap(state.index_sequence[i - 1], state.index_sequence[j]);
    }
    return state;
}

NO_SANITIZE("unsigned-integer-overflow")
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

/// Merge data from `b` and `a`.
/// Assuming `a` has high entropy, only do ops that retain entropy even if `b`
/// has low entropy (i.e. do not do `a & b`).
NO_SANITIZE("unsigned-integer-overflow")
void random_merge(uint32_t& a, uint32_t b, uint32_t selector) noexcept
{
    switch (selector % 4)
    {
    case 0:
        a = (a * 33) + b;
        break;
    case 1:
        a = (a ^ b) * 33;
        break;
    case 2:
        a = rotl32(a, ((selector >> 16) % 32)) ^ b;
        break;
    case 3:
        a = rotr32(a, ((selector >> 16) % 32)) ^ b;
        break;
    }
}

void build_l1_cache(uint32_t cache[l1_cache_num_items], const epoch_context& context) noexcept
{
    static constexpr uint32_t num_items = l1_cache_size / sizeof(hash2048);
    for (uint32_t i = 0; i < num_items; ++i)
    {
        auto data = calculate_dataset_item_2048(context, i);
        static constexpr size_t num_words = sizeof(data) / sizeof(cache[0]);
        for (size_t j = 0; j < num_words; ++j)
            cache[i * num_words + j] = le::uint32(data.word32s[j]);
    }
}

}  // namespace progpow
