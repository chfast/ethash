// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash.hpp>
#include "ethash-internal.hpp"

#include <ethash-buildinfo.h>

#include "keccak.hpp"
#include "params.hpp"
#include "utils.hpp"

namespace ethash
{
namespace
{
uint32_t load_uint32(const hash512& item)
{
    // FIXME: Add support for big-endian architectures.
    return static_cast<uint32_t>(item.words[0]);
}
}


uint64_t calculate_light_cache_size(uint32_t epoch_number) noexcept
{
    // FIXME: Handle overflow.
    uint64_t size_upper_bound = light_cache_init_size + uint64_t(epoch_number) * light_cache_growth;
    uint64_t num_items_upper_bound = size_upper_bound / mixhash_size;
    uint64_t num_items = find_largest_prime(num_items_upper_bound);
    return num_items * mixhash_size;  //< This cannot overflow.
}

uint64_t calculate_full_dataset_size(uint32_t epoch_number) noexcept
{
    // FIXME: Handle overflow.
    uint64_t size_upper_bound =
        full_dataset_init_size + uint64_t(epoch_number) * full_dataset_growth;
    uint64_t num_items_upper_bound = size_upper_bound / mix_size;
    uint64_t num_items = find_largest_prime(num_items_upper_bound);
    return num_items * mix_size;  //< This cannot overflow.
}

hash256 calculate_seed(uint32_t epoch_number) noexcept
{
    hash256 seed;
    for (size_t i = 0; i < epoch_number; ++i)
        seed = keccak256(seed.bytes, sizeof(seed));
    return seed;
}

std::vector<hash512> make_light_cache(size_t size, const hash256& seed)
{
    size_t n = size / sizeof(hash512);

    hash512 item = keccak512(seed.bytes, sizeof(seed));
    std::vector<hash512> cache;
    cache.reserve(n);
    cache.emplace_back(item);
    for (size_t i = 1; i < n; ++i)
    {
        item = keccak512(item.bytes, sizeof(item));
        cache.emplace_back(item);
    }

    for (size_t q = 0; q < light_cache_rounds; ++q)
    {
        for (size_t i = 0; i < n; ++i)
        {
            // Fist index: 4 first bytes of the item as little-endian integer.
            size_t t = load_uint32(cache[i]);
            size_t v = t % n;

            // Second index.
            size_t w = (n + i - 1) % n;

            hash512 xored = bitwise_xor(cache[v], cache[w]);
            cache[i] = keccak512(xored.bytes, sizeof(xored));
        }
    }

    return cache;
}

const char* version() noexcept
{
    return ethash_get_buildinfo()->project_version;
}
}
