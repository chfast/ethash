// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include "kiss99.hpp"
#include <ethash/ethash.hpp>
#include <array>

namespace progpow
{
using namespace ethash;

static constexpr int period_length = 50;
static constexpr uint32_t num_regs = 32;
static constexpr size_t l1_cache_size = 16 * 1024;
static constexpr size_t l1_cache_num_items = l1_cache_size / sizeof(uint32_t);

/// A variant of Keccak hash function for ProgPoW.
///
/// This Keccak hash function uses 800-bit permutation (Keccak-f[800]) with 576 bitrate.
/// It take exactly 576 bits of input (split across 3 arguments) and adds no padding.
///
/// @param header_hash  The 256-bit header hash.
/// @param nonce        The 64-bit nonce.
/// @param mix_hash     Additional 256-bits of data.
/// @return             The 256-bit output of the hash function.
hash256 keccak_progpow_256(
    const hash256& header_hash, uint64_t nonce, const hash256& mix_hash) noexcept;

/// The same as keccak_progpow_256() but uses null mix
/// and returns top 64 bits of the output being a big-endian prefix of the 256-bit hash.
uint64_t keccak_progpow_64(const hash256& header_hash, uint64_t nonce) noexcept;

/// ProgPoW mix RNG state.
///
/// Encapsulates the state of the random number generator used in computing ProgPoW mix.
/// This includes the state of the KISS99 RNG and the precomputed random permutation of the
/// sequence of mix item indexes.
class mix_rng_state
{
public:
    explicit mix_rng_state(uint64_t seed) noexcept;

    uint32_t next_dst() noexcept { return dst_seq[(dst_counter++) % num_regs]; }
    uint32_t next_src() noexcept { return src_seq[(src_counter++) % num_regs]; }

    kiss99 rng;

private:
    size_t dst_counter = 0;
    std::array<uint32_t, num_regs> dst_seq;
    size_t src_counter = 0;
    std::array<uint32_t, num_regs> src_seq;
};

}  // namespace progpow
