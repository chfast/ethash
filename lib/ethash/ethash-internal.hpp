// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// Contains non-public ethash function declarations.

#pragma once

#include "types.hpp"

namespace ethash
{

hash256 calculate_seed(uint32_t epoch_number) noexcept;

}