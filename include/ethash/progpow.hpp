// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// @file
///
/// ProgPoW API
///
/// This file provides the public API for ProgPoW as the Ethash API extension.

#include <ethash/ethash.hpp>

namespace progpow
{
using namespace ethash;  // Include ethash namespace.

result hash(const epoch_context& context, int block_number, const hash256& header_hash,
    uint64_t nonce) noexcept;
}
