/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018-2019 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0.
 */

#pragma once

#include "../support/attributes.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** FNV 32-bit prime. */
static const uint32_t fnv_prime = 0x01000193;

/**
 * The implementation of FNV-1 hash.
 *
 * See https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV-1_hash.
 */
NO_SANITIZE("unsigned-integer-overflow")
static inline uint32_t fnv1(uint32_t u, uint32_t v) noexcept
{
    return (u * fnv_prime) ^ v;
}

#ifdef __cplusplus
}
#endif
