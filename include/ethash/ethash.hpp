// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <cstdint>
#include <vector>

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

union hash512
{
    uint64_t words[8] = {
        0,
    };
    uint32_t half_words[16];
    char bytes[64];
};

using light_cache = std::vector<hash512>;

struct epoch_context
{
    light_cache cache;
    size_t full_dataset_size;

    explicit epoch_context(uint32_t epoch_number);
};

uint64_t search_light(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);
}
