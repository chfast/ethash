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

struct epoch_context;

epoch_context* create_epoch_context(uint32_t epoch_number) noexcept;

void destroy_epoch_context(epoch_context* context) noexcept;

void init_full_dataset(epoch_context& context);

uint64_t search_light(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);

uint64_t search(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);
}
