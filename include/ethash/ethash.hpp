// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <cstdint>

namespace ethash
{

uint64_t calculate_light_cache_size(uint32_t epoch_number) noexcept;

uint64_t calculate_full_dataset_size(uint32_t epoch_number) noexcept;

}
