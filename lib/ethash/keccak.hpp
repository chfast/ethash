// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <ethash/ethash.hpp>

#include <cstring>

extern "C" void ethash_keccakf(uint64_t* state) noexcept;

namespace ethash
{

inline void keccak_load_block_into_state(uint64_t* state, const uint64_t* block, size_t block_size) noexcept
{
    for (size_t i = 0; i < (block_size / sizeof(uint64_t)); ++i)
        state[i] ^= block[i];  // TODO: Add BE support.
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
        keccak_load_block_into_state(state, data, block_size);
        ethash_keccakf(state);
        data += block_words;
        size -= block_words;
    }

    // Final block:
    uint64_t block[block_words] = {};
    // Weirdly, GCC and clang are able to optimize memcpy better than for loop.
    std::memcpy(block, data, size * sizeof(uint64_t));

    // Padding:
    auto block_bytes = reinterpret_cast<unsigned char*>(block);
    block_bytes[size * sizeof(uint64_t)] = 0x01;
    block_bytes[block_size - 1] = 0x80;

    keccak_load_block_into_state(state, block, block_size);
    ethash_keccakf(state);

    typename hash_selector<bits>::type hash;
    std::memcpy(hash.bytes, state, sizeof(hash));
    return hash;
}

inline hash256 keccak256(const hash256& input) noexcept
{
    return keccak<256>(input.words, sizeof(input) / sizeof(uint64_t));
}

inline hash512 keccak512(const hash256& input) noexcept
{
    return keccak<512>(input.words, sizeof(input) / sizeof(uint64_t));
}

inline hash512 keccak512(const hash512& input) noexcept
{
    return keccak<512>(input.words, sizeof(input) / sizeof(uint64_t));
}

inline hash1024 double_keccak(const hash1024& input) noexcept
{
    static constexpr size_t bits = 512;
    static constexpr size_t size = sizeof(input.hashes[0]) / sizeof(uint64_t);
    static constexpr size_t block_size = (1600 - bits * 2) / 8;
    static constexpr size_t block_words = block_size / sizeof(uint64_t);

    auto data0 = &input.hashes[0].bytes;
    auto data1 = &input.hashes[1].bytes;

    uint64_t state0[25] = {};
    uint64_t state1[25] = {};

//    while (size >= block_words)
//    {
//        keccak_load_block_into_state(state, data, block_size);
//        ethash_keccakf(state);
//        data += block_words;
//        size -= block_words;
//    }

    // Final block:
    uint64_t block0[block_words] = {};
    uint64_t block1[block_words] = {};
    std::memcpy(block0, data0, size * sizeof(uint64_t));
    std::memcpy(block1, data1, size * sizeof(uint64_t));

    // Padding:
    auto block_bytes0 = reinterpret_cast<unsigned char*>(block0);
    block_bytes0[size * sizeof(uint64_t)] = 0x01;
    block_bytes0[block_size - 1] = 0x80;
    auto block_bytes1 = reinterpret_cast<unsigned char*>(block1);
    block_bytes1[size * sizeof(uint64_t)] = 0x01;
    block_bytes1[block_size - 1] = 0x80;

    keccak_load_block_into_state(state0, block0, block_size);
    keccak_load_block_into_state(state1, block1, block_size);
    ethash_keccakf(state0);
    ethash_keccakf(state1);

    hash1024 hash;
    std::memcpy(hash.hashes[0].bytes, state0, size * sizeof(uint64_t));
    std::memcpy(hash.hashes[1].bytes, state1, size * sizeof(uint64_t));
    return hash;
}

}
