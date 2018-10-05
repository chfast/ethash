/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#pragma once

#include "builtins.h"

#include <stdint.h>

#if __clang__
#define ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW \
    __attribute__((no_sanitize("unsigned-integer-overflow")))
#else
#define ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t rotl32(uint32_t n, unsigned int c)
{
    const unsigned int mask = 31;

    c &= mask;
    unsigned int neg_c = (unsigned int)(-(int)c);
    return (n << c) | (n >> (neg_c & mask));
}

static inline uint32_t rotr32(uint32_t n, unsigned int c)
{
    const unsigned int mask = 31;

    c &= mask;
    unsigned int neg_c = (unsigned int)(-(int)c);
    return (n >> c) | (n << (neg_c & mask));
}

static inline uint32_t clz32(uint32_t x)
{
    return x ? (uint32_t)__builtin_clz(x) : 32;
}

static inline uint32_t popcount32(uint32_t x)
{
    return (uint32_t)__builtin_popcount(x);
}

static inline uint32_t mul_hi32(uint32_t x, uint32_t y)
{
    return (uint32_t)(((uint64_t)x * (uint64_t)y) >> 32);
}

/**
 * The implementation of FNV-1 hash.
 *
 * See https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash.
 */
ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW
static inline uint32_t fnv1(uint32_t u, uint32_t v) noexcept
{
    return (u * 0x01000193) ^ v;
}

#ifdef __cplusplus
}
#endif
