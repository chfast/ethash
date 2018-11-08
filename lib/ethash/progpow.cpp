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
static constexpr size_t num_lines = 16;
static constexpr int num_cache_accesses = 8;
static constexpr int num_math_operations = 8;

hash256 keccak_progpow_256(
    const hash256& header_hash, uint64_t nonce, const hash256& mix_hash) noexcept
{
    static constexpr size_t num_words =
        sizeof(header_hash.word32s) / sizeof(header_hash.word32s[0]);

    uint32_t state[25] = {};

    size_t i;
    for (i = 0; i < num_words; ++i)
        state[i] = le::uint32(header_hash.word32s[i]);

    state[i++] = static_cast<uint32_t>(nonce);
    state[i++] = static_cast<uint32_t>(nonce >> 32);

    for (uint32_t mix_word : mix_hash.word32s)
        state[i++] = mix_word;

    ethash_keccakf800(state);

    hash256 output;
    for (i = 0; i < num_words; ++i)
        output.word32s[i] = le::uint32(state[i]);
    return output;
}

uint64_t keccak_progpow_64(
    const hash256& header_hash, uint64_t nonce, const hash256& mix_hash) noexcept
{
    const hash256 h = keccak_progpow_256(header_hash, nonce, mix_hash);
    return be::uint64(h.word64s[0]);
}

mix_rng_state::mix_rng_state(uint64_t seed) noexcept
{
    const uint32_t seed_lo = static_cast<uint32_t>(seed);
    const uint32_t seed_hi = static_cast<uint32_t>(seed >> 32);

    uint32_t z = fnv1a(fnv_offset_basis, seed_lo);
    uint32_t w = fnv1a(z, seed_hi);
    uint32_t jsr = fnv1a(w, seed_lo);
    uint32_t jcong = fnv1a(jsr, seed_hi);

    rng = kiss99{z, w, jsr, jcong};

    // Create random permutations of mix destinations / sources.
    // Uses Fisher–Yates shuffle.
    for (uint32_t i = 0; i < num_regs; ++i)
    {
        dst_seq[i] = i;
        src_seq[i] = i;
    }

    for (uint32_t i = num_regs; i > 1; --i)
    {
        std::swap(dst_seq[i - 1], dst_seq[rng() % i]);
        std::swap(src_seq[i - 1], src_seq[rng() % i]);
    }
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

static void round(
    const epoch_context& context, uint32_t r, mix_array& mix, const mix_rng_state& state_reference)
{
    const uint32_t num_items = static_cast<uint32_t>(context.full_dataset_num_items / 2);
    const uint32_t item_index = mix[r % num_lines][0] % num_items;
    const hash2048 item = calculate_dataset_item_2048(context, item_index);

    // Lanes can execute in parallel and will be convergent
    for (size_t l = 0; l < num_lines; l++)
    {
        mix_rng_state state{state_reference};

        int max_i = std::max(num_cache_accesses, num_math_operations);
        for (int i = 0; i < max_i; i++)
        {
            if (i < num_cache_accesses)
            {
                // Random access to cached memory.
                const auto src = state.next_src();
                const auto dst = state.next_dst();
                const auto sel = state.rng();
                const size_t offset = mix[l][src] % l1_cache_num_items;
                random_merge(mix[l][dst], context.l1_cache[offset], sel);
            }
            if (i < num_math_operations)
            {
                // Random Math
                auto src1 = state.rng() % num_regs;
                auto src2 = state.rng() % num_regs;
                auto sel1 = state.rng();
                auto dst = state.next_dst();
                auto sel2 = state.rng();
                uint32_t data32 = random_math(mix[l][src1], mix[l][src2], sel1);
                random_merge(mix[l][dst], data32, sel2);
            }
        }

        static constexpr size_t num_words = sizeof(item) / (sizeof(uint32_t) * num_lines);
        for (size_t i = 0; i < num_words; i++)
        {
            const auto word = le::uint32(item.word32s[l * num_words + i]);
            const auto dst = i == 0 ? 0 : state.next_dst();
            random_merge(mix[l][dst], word, state.rng());
        }
    }
}

mix_array init_mix(uint64_t seed)
{
    const uint32_t z = fnv1a(fnv_offset_basis, static_cast<uint32_t>(seed));
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

result hash(const epoch_context& context, int block_number, const hash256& header_hash,
    uint64_t nonce) noexcept
{
    hash256 mix_hash{};
    uint64_t seed = keccak_progpow_64(header_hash, nonce, mix_hash);

    auto mix = init_mix(seed);
    mix_rng_state state{uint64_t(block_number / period_length)};

    // execute the randomly generated inner loop
    for (uint32_t i = 0; i < 64; i++)
        round(context, i, mix, state);

    // Reduce mix data to a single per-lane result
    uint32_t lane_hash[num_lines];
    for (size_t l = 0; l < num_lines; l++)
    {
        lane_hash[l] = fnv_offset_basis;
        for (uint32_t i = 0; i < num_regs; i++)
            lane_hash[l] = fnv1a(lane_hash[l], mix[l][i]);
    }
    // Reduce all lanes to a single 256-bit result
    static constexpr size_t num_words = sizeof(hash256) / sizeof(uint32_t);
    for (uint32_t& w : mix_hash.word32s)
        w = fnv_offset_basis;
    for (size_t l = 0; l < num_lines; l++)
        mix_hash.word32s[l % num_words] = fnv1a(mix_hash.word32s[l % num_words], lane_hash[l]);

    const hash256 final_hash = keccak_progpow_256(header_hash, seed, mix_hash);
    mix_hash = le::uint32s(mix_hash);
    return {final_hash, mix_hash};
}

}  // namespace progpow
