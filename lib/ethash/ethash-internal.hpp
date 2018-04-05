// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// @file
/// Contains declarations of internal ethash functions to allow them to be
/// unit-tested.

#pragma once

#include <ethash/ethash.hpp>

#include <memory>
#include <vector>

namespace ethash
{
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
    hash512 hashes[2];
    uint64_t words[16];
    uint32_t hwords[32];
    char bytes[128];

    constexpr hash1024() : hashes{{}, {}} {}
};

size_t calculate_light_cache_num_items(int epoch_number) noexcept;

uint64_t calculate_full_dataset_size(int epoch_number) noexcept;

hash256 calculate_seed(int epoch_number) noexcept;

hash512* make_light_cache(size_t num_items, const hash256& seed);

hash1024 calculate_dataset_item(const ethash_epoch_context& context, size_t index) noexcept;

hash512 calculate_dataset_item_partial(const hash512* cache, size_t num_cache_items, size_t index) noexcept;
}

extern "C" struct ethash_epoch_context
{
    int epoch_number = -1;
    size_t light_cache_num_items = 0;
    ethash::hash512* light_cache = nullptr;
    size_t full_dataset_size = 0;
    ethash::hash1024* full_dataset = nullptr;
};