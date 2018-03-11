// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "ethash-internal.hpp"

#include <cassert>
#include <cstring>
#include <functional>
#include <limits>

#include "keccak.hpp"
#include "params.hpp"
#include "utils.hpp"
#include <ethash-buildinfo.h>

#include <iostream>


namespace ethash
{
namespace
{
inline uint32_t load_uint32(const hash512& item)
{
    // TODO: Add support for big-endian architectures.
    return static_cast<uint32_t>(item.words[0]);
}

inline uint32_t fnv(uint32_t u, uint32_t v)
{
    return (u * 0x01000193) ^ v;
}

inline hash512 fnv(const hash512& u, const hash512& v)
{
    hash512 r;
    for (size_t i = 0; i < sizeof(r) / sizeof(r.half_words[0]); ++i)
        r.half_words[i] = fnv(u.half_words[i], v.half_words[i]);

    return r;
}
}


epoch_context::epoch_context(uint32_t epoch_number)
{
    size_t cache_size = calculate_light_cache_size(epoch_number);
    hash256 seed = calculate_seed(epoch_number);
    cache = make_light_cache(cache_size, seed);
    full_dataset_size = calculate_full_dataset_size(epoch_number);
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
        seed = keccak256(seed);
    return seed;
}

light_cache make_light_cache(size_t size, const hash256& seed)
{
    size_t n = size / sizeof(hash512);

    hash512 item = keccak512(seed);
    light_cache cache;
    cache.reserve(n);
    cache.emplace_back(item);
    for (size_t i = 1; i < n; ++i)
    {
        item = keccak512(item);
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
            cache[i] = keccak512(xored);
        }
    }

    return cache;
}

hash512 calculate_full_dataset_item(const light_cache& cache, size_t index)
{
    assert(cache.size() <= std::numeric_limits<uint32_t>::max());

    static constexpr size_t num_half_words = sizeof(hash512) / sizeof(uint32_t);
    const size_t num_cache_items = cache.size();

    const uint32_t init = static_cast<uint32_t>(index);

    hash512 mix = cache[index % num_cache_items];
    mix.half_words[0] ^= init;  // TODO: Add BE support.

    mix = keccak512(mix);

    for (uint32_t j = 0; j < full_dataset_item_parents; ++j)
    {
        uint32_t t = fnv(init ^ j, mix.half_words[j % num_half_words]);
        size_t parent_index = t % num_cache_items;
        mix = fnv(mix, cache[parent_index]);
    }

    return keccak512(mix);
}

void init_full_dataset(epoch_context& context)
{
    assert(context.full_dataset == nullptr);
    const size_t num_items = context.full_dataset_size / sizeof(hash512);
    context.full_dataset.reset(new hash512[num_items]);
}

namespace
{
union mix_t
{
    hash512 hashes[2] = {{}, {}};
    uint32_t hwords[32];
    char bytes[128];
};

constexpr size_t mix_hashes = sizeof(mix_t) / sizeof(hash512);
constexpr size_t mix_hwords = sizeof(mix_t) / sizeof(uint32_t);

using lookup_fn = std::function<mix_t(const epoch_context&, size_t)>;

inline hash256 hash_kernel(const epoch_context& context, const hash256& header_hash, uint64_t nonce, lookup_fn lookup)
{
    const auto n = context.full_dataset_size;

    uint64_t init_data[5];
    for (size_t i = 0; i < sizeof(header_hash) / sizeof(uint64_t); ++i)
        init_data[i] = header_hash.words[i];
    init_data[4] = nonce;
    hash512 s = keccak<512>(init_data, 5);
    const uint32_t s_init = s.half_words[0];

    mix_t mix;
    std::memcpy(&mix.bytes[0], s.bytes, sizeof(s));
    std::memcpy(&mix.bytes[sizeof(mix) / 2], s.bytes, sizeof(s));

    for (uint32_t i = 0; i < 64; ++i)
    {
        auto p = fnv(i ^ s_init, mix.hwords[i % mix_hwords]) % (n / sizeof(mix)) * mix_hashes;
        mix_t newdata = lookup(context, p);

        for (size_t j = 0; j < mix_hwords; ++j)
            mix.hwords[j] = fnv(mix.hwords[j], newdata.hwords[j]);
    }

    hash256 cmix;
    for (size_t i = 0; i < mix_hwords; i += 4)
    {
        uint32_t h1 = fnv(mix.hwords[i], mix.hwords[i + 1]);
        uint32_t h2 = fnv(h1, mix.hwords[i + 2]);
        uint32_t h3 = fnv(h2, mix.hwords[i + 3]);
        cmix.hwords[i / 4] = h3;
    }

    uint64_t final_data[12];
    std::memcpy(&final_data[0], s.bytes, sizeof(s));
    std::memcpy(&final_data[8], cmix.bytes, sizeof(cmix));
    return keccak<256>(final_data, 12);
}
}

hash256 hash_light(const epoch_context& context, const hash256& header_hash, uint64_t nonce)
{
    static constexpr auto light_lookup = [](const epoch_context& context, size_t index) {
        mix_t data;
        data.hashes[0] = calculate_full_dataset_item(context.cache, index);
        data.hashes[1] = calculate_full_dataset_item(context.cache, index + 1);
        return data;
    };

    return hash_kernel(context, header_hash, nonce, light_lookup);
}

hash256 hash(const epoch_context& context, const hash256& header_hash, uint64_t nonce)
{
    assert(context.full_dataset != nullptr);

    static constexpr auto lazy_lookup = [](const epoch_context& context, size_t index) {
        // TODO: not thead-safe.
        // This is not thread-safe, add atomics here. Thread sanitizer is not able to report this.
        hash512& item0 = context.full_dataset[index];
        if (item0.words[0] == 0)
            item0 = calculate_full_dataset_item(context.cache, index);

        hash512& item1 = context.full_dataset[index + 1];
        if (item1.words[0] == 0)
            item1 = calculate_full_dataset_item(context.cache, index + 1);

        mix_t data;
        data.hashes[0] = item0;
        data.hashes[1] = item1;
        return data;
    };

    return hash_kernel(context, header_hash, nonce, lazy_lookup);
}

uint64_t search_light(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations)
{
    const uint64_t end_nonce = start_nonce + iterations;
    for (uint64_t nonce = start_nonce; nonce < end_nonce; ++nonce)
    {
        hash256 h = hash_light(context, header_hash, nonce);
        if (h.words[0] < target)
            return nonce;
    }
    return 0;
}

uint64_t search(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations)
{
    const uint64_t end_nonce = start_nonce + iterations;
    for (uint64_t nonce = start_nonce; nonce < end_nonce; ++nonce)
    {
        hash256 h = hash(context, header_hash, nonce);
        if (h.words[0] < target)
            return nonce;
    }
    return 0;
}

const char* version() noexcept
{
    return ethash_get_buildinfo()->project_version;
}
}
