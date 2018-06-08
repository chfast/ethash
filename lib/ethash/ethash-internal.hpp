// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// @file
/// Contains declarations of internal ethash functions to allow them to be
/// unit-tested.

#pragma once

#include <ethash/ethash.hpp>

#include "endianness.hpp"

#include <memory>
#include <vector>

extern "C" struct ethash_epoch_context_full : ethash_epoch_context
{
    ethash_hash2048* full_dataset;
	ethash_hash32* full_l1_dataset;

    constexpr ethash_epoch_context_full(int epoch_number, int light_cache_num_items,
        const ethash_hash512* light_cache, int full_dataset_num_items,
        ethash_hash2048* full_dataset, ethash_hash32* full_l1_dataset) noexcept
      : ethash_epoch_context{epoch_number, light_cache_num_items, light_cache,
            full_dataset_num_items},
        full_dataset{full_dataset}, full_l1_dataset{ full_l1_dataset }
    {}
};

namespace ethash
{
inline bool is_less_or_equal(const hash256& a, const hash256& b) noexcept
{
    for (size_t i = 0; i < (sizeof(a) / sizeof(a.words[0])); ++i)
    {
        if (from_be(a.words[i]) > from_be(b.words[i]))
            return false;
        if (from_be(a.words[i]) < from_be(b.words[i]))
            return true;
    }
    return true;
}

void build_light_cache(hash512 cache[], int num_items, const hash256& seed) noexcept;

hash1024 calculate_dataset_item(const epoch_context& context, uint32_t index) noexcept;

hash2048 calculate_dataset_item_progpow(const epoch_context& context, uint32_t index) noexcept;

}  // namespace ethash
