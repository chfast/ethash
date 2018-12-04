// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include "kiss99.hpp"
#include <ethash/ethash.hpp>
#include <array>

namespace progpow
{
using namespace ethash;

static constexpr int period_length = 50;
static constexpr uint32_t num_regs = 32;
static constexpr size_t l1_cache_size = 16 * 1024;
static constexpr size_t l1_cache_num_items = l1_cache_size / sizeof(uint32_t);
}  // namespace progpow
