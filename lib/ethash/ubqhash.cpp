// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ubqhash.hpp>

#include "ethash-internal.hpp"
#include <ethash/blake2b.h>

namespace ubqhash
{
namespace
{
void build_light_cache(hash512 cache[], int num_items, const hash256& seed) noexcept
{
    return generic::build_light_cache(ethash_blake2b_512, cache, num_items, seed);
}
}  // namespace

epoch_context_ptr create_epoch_context(int epoch_number) noexcept
{
    auto* context = generic::create_epoch_context(build_light_cache, epoch_number, false);
    return {context, ethash_destroy_epoch_context};
}

}  // namespace ubqhash
