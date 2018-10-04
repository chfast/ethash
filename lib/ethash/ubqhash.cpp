// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "ubqhash-internal.hpp"

#include <ethash/blake2b.h>

namespace ubqhash
{
void build_light_cache(hash512 cache[], int num_items, const hash256& seed) noexcept
{
    return ethash::generic::build_light_cache(ethash_blake2b_512, cache, num_items, seed);
}
}  // namespace ubqhash
