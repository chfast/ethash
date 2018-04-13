// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <cstdint>
#include <cstddef>

namespace ethash
{
constexpr static int light_cache_init_size = 1 << 24;
constexpr static int light_cache_growth = 1 << 17;
constexpr static int light_cache_rounds = 3;
constexpr static int full_dataset_init_size = 1 << 30;
constexpr static int full_dataset_growth = 1 << 23;
constexpr static uint32_t full_dataset_item_parents = 256;
}