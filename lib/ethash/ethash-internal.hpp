// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// Contains non-public ethash function declarations.

#pragma once

#include "types.hpp"

#include <vector>
#include <cstddef>

namespace ethash
{

using light_cache = std::vector<hash512>;

struct epoch_context
{
    light_cache cache;
    size_t full_dataset_size;

    explicit epoch_context(uint32_t epoch_number);
};

hash256 calculate_seed(uint32_t epoch_number) noexcept;

light_cache make_light_cache(size_t size, const hash256& seed);

hash512 calculate_full_dataset_item(const light_cache& cache, uint32_t index);

hash256 calculate_hash(const epoch_context& context, const hash256& header_hash, uint64_t nonce);

const char* version() noexcept;

}