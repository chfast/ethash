// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <cstdint>
#include <cstddef>

namespace ethash
{
union hash256
{
    uint64_t words[4] = {
        0,
    };
    uint32_t hwords[8];
    char bytes[32];
};

struct result
{
    hash256 final_hash;
    hash256 mix_hash;
};

struct epoch_context;

epoch_context* create_epoch_context(uint32_t epoch_number) noexcept;

void destroy_epoch_context(epoch_context* context) noexcept;


/// Init full dataset in the epoch context.
///
/// This allocates the memory for the full dataset and inits dataset items
/// marked as "not-generated".
///
/// @param context  The epoch context.
/// @return  Returns true if memory allocations succeeded, false otherwise.
bool init_full_dataset(epoch_context& context) noexcept;


result hash_light(const epoch_context& context, const hash256& header_hash, uint64_t nonce);

result hash(const epoch_context& context, const hash256& header_hash, uint64_t nonce);

uint64_t search_light(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);

uint64_t search(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);
}
