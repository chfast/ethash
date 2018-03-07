// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// Contains non-public ethash function declarations.

#pragma once

#include <ethash/ethash.hpp>

#include <vector>
#include <cstddef>

namespace ethash
{

uint64_t calculate_light_cache_size(uint32_t epoch_number) noexcept;

uint64_t calculate_full_dataset_size(uint32_t epoch_number) noexcept;

hash256 calculate_seed(uint32_t epoch_number) noexcept;

light_cache make_light_cache(size_t size, const hash256& seed);

hash512 calculate_full_dataset_item(const light_cache& cache, size_t index);

hash256 hash_light(const epoch_context& context, const hash256& header_hash, uint64_t nonce);

hash256 hash(const epoch_context& context, const hash256& header_hash, uint64_t nonce);

const char* version() noexcept;

}