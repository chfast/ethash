// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "progpow-internal.hpp"

#include "endianness.hpp"

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
    const hash256 &header_hash, uint64_t nonce, const uint32_t *extra) noexcept
{
    const hash256 h = keccak_progpow_256(header_hash, nonce, extra);
    return (uint64_t(h.hwords[0]) << 32) | h.hwords[1];
}

}  // namespace progpow
