// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "ethash-internal.hpp"
#include <ethash/ethash.hpp>

#include <cassert>
#include <cstring>
#include <limits>

#include "keccak.hpp"
#include "params.hpp"
#include "utils.hpp"
#include <ethash-buildinfo.h>


namespace ethash
{
namespace
{
inline uint32_t load_uint32(const hash512& item)
{
    // FIXME: Add support for big-endian architectures.
    return static_cast<uint32_t>(item.words[0]);
}

inline uint32_t fnv(uint32_t u, uint32_t v)
{
    return (u * 0x01000193) ^ v;
}

inline uint64_t fnv(uint64_t u, uint64_t v)
{
    uint32_t ul = static_cast<uint32_t>(u);
    uint32_t uh = static_cast<uint32_t>(u >> 32);
    uint32_t vl = static_cast<uint32_t>(v);
    uint32_t vh = static_cast<uint32_t>(v >> 32);

    uint64_t l = fnv(ul, vl);
    uint64_t h = fnv(uh, vh);

    return (h << 32) | l;
}

inline hash512 fnv(const hash512& u, const hash512& v)
{
    hash512 r;
    for (size_t i = 0; i < sizeof(r) / sizeof(r.half_words[0]); ++i)
        r.half_words[i] = fnv(u.half_words[i], v.half_words[i]);

    return r;
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

light_cache make_light_cache(size_t size, const hash256& seed)
{
    size_t n = size / sizeof(hash512);

    hash512 item = keccak512(seed.bytes, sizeof(seed));
    light_cache cache;
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

hash512 calculate_full_dataset_item(const light_cache& cache, uint32_t index)
{
    assert(cache.size() <= std::numeric_limits<uint32_t>::max());

    const size_t n = cache.size();
    static constexpr uint32_t num_half_words = sizeof(hash512) / sizeof(uint32_t);

    hash512 mix = cache[index % n];
    mix.half_words[0] ^= index;  // TODO: Add BE support.

    mix = keccak512(mix.bytes, sizeof(mix));

    for (uint32_t j = 0; j < full_dataset_item_parents; ++j)
    {
        uint32_t t = fnv(index ^ j, mix.half_words[j % num_half_words]);
        size_t parent_index = t % n;
        mix = fnv(mix, cache[parent_index]);
    }

    return keccak512(mix.bytes, sizeof(mix));
}

hash256 calculate_hash(
    uint32_t epoch, const light_cache& cache, const hash256& header_hash, uint64_t nonce)
{
    const auto n = calculate_full_dataset_size(epoch);
    nonce = __builtin_bswap64(nonce);
    char init_bytes[sizeof(header_hash) + sizeof(nonce)];
    std::memcpy(&init_bytes[0], header_hash.bytes, sizeof(header_hash));
    std::memcpy(&init_bytes[sizeof(header_hash)], &nonce, sizeof(nonce));
    hash512 s = keccak512(init_bytes, sizeof(init_bytes));
    const uint32_t s_init = s.half_words[0];

    union mix_t
    {
        hash512 hashes[2] = {{}, {}};
        uint32_t hwords[32];
        char bytes[128];
    };
    static constexpr size_t mix_hashes = sizeof(mix_t) / sizeof(hash512);
    static constexpr size_t mix_hwords = sizeof(mix_t) / sizeof(uint32_t);

    mix_t mix;
    std::memcpy(&mix.bytes[0], s.bytes, sizeof(s));
    std::memcpy(&mix.bytes[sizeof(mix) / 2], s.bytes, sizeof(s));

    for (uint32_t i = 0; i < 64; ++i)
    {
        auto p = fnv(i ^ s_init, mix.hwords[i % mix_hwords]) % (n / sizeof(mix)) * mix_hashes;
        mix_t newdata;
        newdata.hashes[0] = calculate_full_dataset_item(cache, static_cast<uint32_t>(p));
        newdata.hashes[1] = calculate_full_dataset_item(cache, static_cast<uint32_t>(p + 1));

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

    char final_data[sizeof(s) + sizeof(cmix)];
    std::memcpy(&final_data[0], s.bytes, sizeof(s));
    std::memcpy(&final_data[sizeof(s)], cmix.bytes, sizeof(cmix));
    return keccak256(final_data, sizeof(final_data));
}


const char* version() noexcept
{
    return ethash_get_buildinfo()->project_version;
}
}
