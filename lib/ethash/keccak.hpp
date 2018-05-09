// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include "endianness.hpp"

#include <ethash/ethash.hpp>
#include <ethash/keccak.h>

#include <cstring>

namespace ethash
{

inline void keccak_load_block_into_state(uint64_t* state, const uint64_t* block, size_t block_words) noexcept
{
    for (size_t i = 0; i < block_words; ++i)
        state[i] ^= fix_endianness(block[i]);
}

template <size_t bits>
struct hash_selector
{
};

template <>
struct hash_selector<256>
{
    using type = hash256;
};
template <>
struct hash_selector<512>
{
    using type = hash512;
};

template <size_t bits>
inline typename hash_selector<bits>::type keccak(const uint64_t* data, size_t size) noexcept
{
    static constexpr size_t block_size = (1600 - bits * 2) / 8;
    static constexpr size_t block_words = block_size / sizeof(uint64_t);

    uint64_t state[25] = {};

    while (size >= block_words)
    {
        keccak_load_block_into_state(state, data, block_words);
        ethash_keccakf1600(state);
        data += block_words;
        size -= block_words;
    }

    // Final block:
    uint64_t block[block_words] = {};
    // Weirdly, GCC and clang are able to optimize memcpy better than for loop.
    // FIXME: This is also UB when data == nullptr.
    std::memcpy(block, data, size * sizeof(uint64_t));

    // Padding:
    auto block_bytes = reinterpret_cast<unsigned char*>(block);
    block_bytes[size * sizeof(uint64_t)] = 0x01;
    // FIXME: Add test case for this padding when bytes API available.
    block_bytes[block_size - 1] |= 0x80;

    keccak_load_block_into_state(state, block, block_words);
    ethash_keccakf1600(state);

    typename hash_selector<bits>::type hash;
    std::memcpy(&hash, state, sizeof(hash));
    return fix_endianness64(hash);
}

inline hash256 keccak256(const hash256& input) noexcept
{
    return ethash_keccak256_word4(input.words);
}

inline hash512 keccak512(const hash256& input) noexcept
{
    // FIXME: Remove this overload.
    return ethash_keccak512(input.bytes, sizeof(input));
}

inline hash512 keccak512(const hash512& input) noexcept
{
    return ethash_keccak512_word8(input.words);
}

inline hash1024 double_keccak(const hash1024& input) noexcept
{
    hash1024 output;
    output.hashes[0] = keccak512(input.hashes[0]);
    output.hashes[1] = keccak512(input.hashes[1]);
    return output;
}

}  // namespace ethash
