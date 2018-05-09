// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include "endianness.hpp"

#include <ethash/keccak.h>
#include <ethash/ethash.hpp>

#include <cstring>

namespace ethash
{
inline hash256 keccak256(const uint8_t* data, size_t size) noexcept
{
    return ethash_keccak256(data, size);
}

inline hash256 keccak256(const hash256& input) noexcept
{
    return ethash_keccak256_word4(input.words);
}

inline hash512 keccak512(const uint8_t* data, size_t size) noexcept
{
    return ethash_keccak512(data, size);
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
