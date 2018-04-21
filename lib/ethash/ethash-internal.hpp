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
hash256 calculate_seed(int epoch_number) noexcept;

void build_light_cache(hash512 cache[], int num_items, const hash256& seed);

hash1024 calculate_dataset_item(const ethash_epoch_context& context, size_t index) noexcept;

hash512 calculate_dataset_item_partial(
    const hash512* cache, int num_cache_items, size_t index) noexcept;

}  // namespace ethash
