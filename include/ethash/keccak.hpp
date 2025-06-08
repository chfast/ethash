// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018-2019 Pawel Bylica.
// Licensed under the Apache License, Version 2.0.

#pragma once

#include <ethash/keccak.h>
#include <ethash/hash_types.hpp>

namespace ethash
{
inline hash256 keccak256(const uint8_t* data, size_t size) noexcept
{
    return ethash_keccak256(data, size);
}

inline hash256 keccak256(const hash256& input) noexcept
{
    return ethash_keccak256_32(input.bytes);
}

inline hash512 keccak512(const uint8_t* data, size_t size) noexcept
{
    return ethash_keccak512(data, size);
}

inline hash512 keccak512(const hash512& input) noexcept
{
    return ethash_keccak512_64(input.bytes);
}

static constexpr auto keccak256_32 = ethash_keccak256_32;
static constexpr auto keccak512_64 = ethash_keccak512_64;

inline void keccak256_init(struct ethash_keccak256_context* ctx) noexcept
{
    ethash_keccak256_init(ctx);
}

inline void keccak256_update(struct ethash_keccak256_context* ctx, const uint8_t* data, size_t size) noexcept
{
    ethash_keccak256_update(ctx, data, size);
}

inline hash256 keccak256_final(struct ethash_keccak256_context* ctx) noexcept
{
    return ethash_keccak256_final(ctx);
}

inline void keccak256_init_2(struct ethash_keccak256_context* ctx) noexcept
{
    ethash_keccak256_init_2(ctx);
}

inline void keccak256_update_2(struct ethash_keccak256_context* ctx, const uint8_t* data, size_t size) noexcept
{
    ethash_keccak256_update_2(ctx, data, size);
}

inline hash256 keccak256_final_2(struct ethash_keccak256_context* ctx) noexcept
{
    return ethash_keccak256_final_2(ctx);
}

}  // namespace ethash
