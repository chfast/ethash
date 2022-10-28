/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018-2019 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0.
 */

#pragma once

#include <ethash/hash_types.h>

#include <stddef.h>

#ifndef __cplusplus
#define noexcept  // Ignore noexcept in C code.
#endif

#ifdef __cplusplus
extern "C" {
#endif

union ethash_hash256 ethash_keccak256(const uint8_t* data, size_t size) noexcept;
union ethash_hash256 ethash_keccak256_32(const uint8_t data[32]) noexcept;
union ethash_hash512 ethash_keccak512(const uint8_t* data, size_t size) noexcept;
union ethash_hash512 ethash_keccak512_64(const uint8_t data[64]) noexcept;

struct ethash_keccak256_context {
    size_t hash_size;
    size_t block_size;
    uint64_t state[25];
    uint64_t* state_iter;
    uint64_t last_word;
    uint8_t* last_word_iter;
};

void ethash_keccak256_init(struct ethash_keccak256_context* ctx) noexcept;
void ethash_keccak256_update(struct ethash_keccak256_context* ctx, const uint8_t* data, size_t size) noexcept;
union ethash_hash256 ethash_keccak256_final(struct ethash_keccak256_context* ctx) noexcept;

#ifdef __cplusplus
}
#endif
