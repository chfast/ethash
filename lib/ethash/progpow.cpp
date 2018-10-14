// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "progpow-internal.hpp"

#include "bit_manipulation.h"
#include "endianness.hpp"
#include "ethash-internal.hpp"
#include "kiss99.hpp"

#include <ethash/keccak.hpp>

namespace progpow
{
static constexpr size_t num_lines = 32;
static constexpr int num_cache_accesses = 8;
static constexpr int num_math_operations = 8;

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

mix_rng_state::mix_rng_state(uint64_t seed) noexcept
{
    const uint32_t seed_lo = static_cast<uint32_t>(seed);
    const uint32_t seed_hi = static_cast<uint32_t>(seed >> 32);

    uint32_t z = fnv1a(0x811c9dc5, seed_lo);
    uint32_t w = fnv1a(z, seed_hi);
    uint32_t jsr = fnv1a(w, seed_lo);
    uint32_t jcong = fnv1a(jsr, seed_hi);

    rng = kiss99{z, w, jsr, jcong};

    // Create a random sequence of mix destinations for merge()
    // guaranteeing every location is touched once.
    // Uses Fisher–Yates shuffle.
    for (uint32_t i = 0; i < num_regs; ++i)
        index_sequence[i] = i;

    for (uint32_t i = num_regs; i > 1; --i)
        std::swap(index_sequence[i - 1], index_sequence[rng() % i]);
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


using mix_array = std::array<std::array<uint32_t, num_regs>, num_lines>;

static void round(const epoch_context& context, uint32_t r, mix_array& mix)
{
    const uint64_t epoch_block_number = uint64_t(context.epoch_number * epoch_length);

    const uint32_t num_items = static_cast<uint32_t>(context.full_dataset_num_items / 2);
    const uint32_t item_index = mix[r % num_lines][0] % num_items;
    const hash2048 item = calculate_dataset_item_2048(context, item_index);

    // Lanes can execute in parallel and will be convergent
    for (size_t l = 0; l < num_lines; l++)
    {
        // initialize the seed and mix destination sequence
        mix_rng_state state{epoch_block_number};  // FIXME: Create once.

        int max_i = std::max(num_cache_accesses, num_math_operations);
        for (int i = 0; i < max_i; i++)
        {
            if (i < num_cache_accesses)
            {
                // Cached memory access, lanes access random location.
                auto src = state.rng() % num_regs;
                auto dst = state.next_index();
                auto sel = state.rng();
                size_t offset = mix[l][src] % l1_cache_num_items;
                random_merge(mix[l][dst], context.l1_cache[offset], sel);
            }
            if (i < num_math_operations)
            {
                // Random Math
                auto src1 = state.rng() % num_regs;
                auto src2 = state.rng() % num_regs;
                auto sel1 = state.rng();
                auto dst = state.next_index();
                auto sel2 = state.rng();
                uint32_t data32 = random_math(mix[l][src1], mix[l][src2], sel1);
                random_merge(mix[l][dst], data32, sel2);
            }
        }
        const uint32_t sel1 = state.rng();
        const uint32_t dst = state.next_index();
        const uint32_t sel2 = state.rng();
        random_merge(mix[l][0], le::uint32(item.word32s[2 * l]), sel1);
        random_merge(mix[l][dst], le::uint32(item.word32s[2 * l + 1]), sel2);
    }
}

mix_array init_mix(uint64_t seed)
{
    const uint32_t z = fnv1a(0x811c9dc5, static_cast<uint32_t>(seed));
    const uint32_t w = fnv1a(z, static_cast<uint32_t>(seed >> 32));

    mix_array mix;
    for (uint32_t l = 0; l < mix.size(); ++l)
    {
        const uint32_t jsr = fnv1a(w, l);
        const uint32_t jcong = fnv1a(jsr, l);
        kiss99 rng{z, w, jsr, jcong};

        for (auto& row : mix[l])
            row = rng();
    }
    return mix;
}

result hash(const epoch_context& context, const hash256& header_hash, uint64_t nonce) noexcept
{
    hash256 mix_hash{};
    uint64_t seed = keccak_progpow_64(header_hash, nonce, mix_hash);

    auto mix = init_mix(seed);

    // execute the randomly generated inner loop
    for (uint32_t i = 0; i < 64; i++)
        round(context, i, mix);

    // Reduce mix data to a single per-lane result
    uint32_t lane_hash[num_lines];
    for (size_t l = 0; l < num_lines; l++)
    {
        lane_hash[l] = 0x811c9dc5;
        for (uint32_t i = 0; i < num_regs; i++)
            lane_hash[l] = fnv1a(lane_hash[l], mix[l][i]);
    }
    // Reduce all lanes to a single 256-bit result
    static constexpr size_t num_words = sizeof(hash256) / sizeof(uint32_t);
    for (uint32_t& w : mix_hash.hwords)
        w = 0x811c9dc5;
    for (size_t l = 0; l < num_lines; l++)
        mix_hash.hwords[l % num_words] = fnv1a(mix_hash.hwords[l % num_words], lane_hash[l]);

    const hash256 final_hash = keccak_progpow_256(header_hash, seed, mix_hash);
    mix_hash = le::uint32s(mix_hash);
    return {final_hash, mix_hash};
}

}  // namespace progpow
