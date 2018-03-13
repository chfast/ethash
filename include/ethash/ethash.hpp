// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <cstdint>
#include <memory>
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

union hash1024
{
    // TODO: Is the array worse than 2 fields for memory aliasing?
    hash512 hashes[2] = {{}, {}};
    uint64_t words[16];
    uint32_t hwords[32];
    char bytes[128];
};

using light_cache = std::vector<hash512>;
using full_dataset_t = std::unique_ptr<hash1024[]>;

struct epoch_context
{
    light_cache cache;
    size_t full_dataset_size;
    full_dataset_t full_dataset;

    explicit epoch_context(uint32_t epoch_number);
};

void init_full_dataset(epoch_context& context);

uint64_t search_light(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);

uint64_t search(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);
}
