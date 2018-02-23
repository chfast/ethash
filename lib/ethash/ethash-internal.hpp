// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// Contains non-public ethash function declarations.

#pragma once

#include "types.hpp"

#include <vector>
#include <cstddef>

namespace ethash
{

hash256 calculate_seed(uint32_t epoch_number) noexcept;

std::vector<hash512> make_light_cache(size_t size, const hash256& seed);

const char* version() noexcept;

}