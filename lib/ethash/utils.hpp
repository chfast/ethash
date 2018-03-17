// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <ethash/ethash.hpp>

#include <cstddef>

namespace ethash
{
bool is_prime(size_t number) noexcept;

/// Finds the largest prime number not greater than the provided upper bound.
///
/// @param upper_bound  The upper bound. MUST be greater than 2.
/// @return  The largest prime number `p` such `p <= upper_bound`.
size_t find_largest_prime(size_t upper_bound) noexcept;
}
