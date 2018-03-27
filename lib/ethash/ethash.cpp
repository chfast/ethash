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

namespace ethash
{
namespace
{
inline uint32_t fnv(uint32_t u, uint32_t v) noexcept
{
    return (u * 0x01000193) ^ v;
}

inline hash512 fnv(const hash512& u, const hash512& v) noexcept
{
    hash512 r;
    for (size_t i = 0; i < sizeof(r) / sizeof(r.half_words[0]); ++i)
        r.half_words[i] = fnv(u.half_words[i], v.half_words[i]);

    return r;
}

hash512 bitwise_xor(const hash512& x, const hash512& y) noexcept
{
    // TODO: Nicely optimized by clang, horribly by GCC.

    hash512 z;
    for (size_t i = 0; i < sizeof(z) / sizeof(z.words[0]); ++i)
        z.words[i] = x.words[i] ^ y.words[i];
    return z;
}
}

epoch_context* create_epoch_context(int epoch_number) noexcept
{
    const size_t cache_size = calculate_light_cache_size(epoch_number);
    hash256 seed = calculate_seed(epoch_number);

    epoch_context* context = new (std::nothrow) epoch_context;
    if (!context)
        return nullptr;  // Signal out-of-memory by returning null pointer.

    context->epoch_number = epoch_number;
    context->cache = make_light_cache(cache_size, seed);
    context->full_dataset_size = calculate_full_dataset_size(epoch_number);
    return context;
}

void destroy_epoch_context(epoch_context* context) noexcept
{
    delete context;
}

uint64_t calculate_light_cache_size(int epoch_number) noexcept
{
    // FIXME: Handle overflow.
    uint64_t size_upper_bound = light_cache_init_size + uint64_t(epoch_number) * light_cache_growth;
    uint64_t num_items_upper_bound = size_upper_bound / mixhash_size;
    uint64_t num_items = find_largest_prime(num_items_upper_bound);
    return num_items * mixhash_size;  //< This cannot overflow.
}

uint64_t calculate_full_dataset_size(int epoch_number) noexcept
{
    static constexpr size_t item_size = sizeof(hash1024);

    // FIXME: Handle overflow.
    uint64_t size_upper_bound =
        full_dataset_init_size + uint64_t(epoch_number) * full_dataset_growth;
    uint64_t num_items_upper_bound = size_upper_bound / item_size;
    uint64_t num_items = find_largest_prime(num_items_upper_bound);
    return num_items * item_size;  //< This cannot overflow.
}

hash256 calculate_seed(int epoch_number) noexcept
{
    hash256 seed;
    for (int i = 0; i < epoch_number; ++i)
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
            size_t t = fix_endianness(cache[i].half_words[0]);
            size_t v = t % n;

            // Second index.
            size_t w = (n + i - 1) % n;

            // Pipelining functions returning structs gives small performance boost.
            cache[i] = keccak512(bitwise_xor(cache[v], cache[w]));
        }
    }

    return cache;
}

/// TODO: Only used in tests or for reference, so can be removed or moved.
hash512 calculate_dataset_item_partial(const light_cache& cache, size_t index) noexcept
{
    assert(cache.size() <= std::numeric_limits<uint32_t>::max());

    static constexpr size_t num_half_words = sizeof(hash512) / sizeof(uint32_t);
    const size_t num_cache_items = cache.size();

    const uint32_t init = static_cast<uint32_t>(index);

    hash512 mix = cache[index % num_cache_items];
    mix.half_words[0] ^= fix_endianness(init);
    mix = keccak512(mix);
    mix = fix_endianness32(mix);  // Covert bytes to 32-bit words.

    for (uint32_t j = 0; j < full_dataset_item_parents; ++j)
    {
        uint32_t t = fnv(init ^ j, mix.half_words[j % num_half_words]);
        size_t parent_index = t % num_cache_items;
        // TODO: Fix endianness when generating the cache item?
        mix = fnv(mix, fix_endianness32(cache[parent_index]));
    }

    return keccak512(fix_endianness32(mix));
}


/// Calculates a full dataset item
///
/// This consist of two 512-bit items produced by calculate_dataset_item_partial().
/// Here the computation is done interleaved for better performance.
hash1024 calculate_dataset_item(const epoch_context& context, size_t index) noexcept
{
    const light_cache& cache = context.cache;

    static constexpr size_t num_half_words = sizeof(hash512) / sizeof(uint32_t);
    const size_t num_cache_items = cache.size();

    const size_t index0 = index * 2;
    const size_t index1 = index * 2 + 1;

    const uint32_t init0 = static_cast<uint32_t>(index0);
    const uint32_t init1 = static_cast<uint32_t>(index1);

    hash1024 mix;
    mix.hashes[0] = cache[index0 % num_cache_items];
    mix.hashes[1] = cache[index1 % num_cache_items];

    mix.hashes[0].half_words[0] ^= fix_endianness(init0);
    mix.hashes[1].half_words[0] ^= fix_endianness(init1);

    mix = double_keccak(mix);

    mix = fix_endianness32(mix);  // Covert bytes to 32-bit words.

    for (uint32_t j = 0; j < full_dataset_item_parents; ++j)
    {
        uint32_t t0 = fnv(init0 ^ j, mix.hashes[0].half_words[j % num_half_words]);
        size_t parent_index0 = t0 % num_cache_items;
        mix.hashes[0] = fnv(mix.hashes[0], fix_endianness32(cache[parent_index0]));

        uint32_t t1 = fnv(init1 ^ j, mix.hashes[1].half_words[j % num_half_words]);
        size_t parent_index1 = t1 % num_cache_items;
        mix.hashes[1] = fnv(mix.hashes[1], fix_endianness32(cache[parent_index1]));
    }

    return double_keccak(fix_endianness32(mix));  // Covert 32-bit words back to bytes and hash.
}

bool init_full_dataset(epoch_context& context) noexcept
{
    assert(context.full_dataset == nullptr);

    const size_t num_items = context.full_dataset_size / sizeof(hash1024);
    context.full_dataset.reset(new (std::nothrow) hash1024[num_items]);
    return context.full_dataset != nullptr;
}

namespace
{

using lookup_fn = std::function<hash1024(const epoch_context&, size_t)>;

inline result hash_kernel(const epoch_context& context, const hash256& header_hash, uint64_t nonce,
    const lookup_fn& lookup)
{
    static constexpr size_t mix_hwords = sizeof(hash1024) / sizeof(uint32_t);
    const size_t num_items = context.full_dataset_size / sizeof(hash1024);

    uint64_t init_data[5];
    std::memcpy(&init_data, &header_hash, sizeof(header_hash));
    init_data[4] = fix_endianness(nonce);

    // Do not convert it to array of native 32-bit words, because bytes are
    // needed in the end.
    const hash512 s = keccak<512>(init_data, 5);

    const uint32_t s_init = fix_endianness(s.half_words[0]);

    hash1024 mix;
    mix.hashes[0] = fix_endianness32(s);
    mix.hashes[1] = fix_endianness32(s);

    for (uint32_t i = 0; i < 64; ++i)
    {
        auto p = fnv(i ^ s_init, mix.hwords[i % mix_hwords]) % num_items;
        hash1024 newdata = fix_endianness32(lookup(context, p));

        for (size_t j = 0; j < mix_hwords; ++j)
            mix.hwords[j] = fnv(mix.hwords[j], newdata.hwords[j]);
    }

    hash256 mix_hash;
    for (size_t i = 0; i < mix_hwords; i += 4)
    {
        uint32_t h1 = fnv(mix.hwords[i], mix.hwords[i + 1]);
        uint32_t h2 = fnv(h1, mix.hwords[i + 2]);
        uint32_t h3 = fnv(h2, mix.hwords[i + 3]);
        mix_hash.hwords[i / 4] = h3;
    }
    mix_hash = fix_endianness32(mix_hash);

    uint64_t final_data[12];
    std::memcpy(&final_data[0], s.bytes, sizeof(s));
    std::memcpy(&final_data[8], mix_hash.bytes, sizeof(mix_hash));
    hash256 final_hash = keccak<256>(final_data, 12);
    return {final_hash, mix_hash};
}
}

result hash_light(const epoch_context& context, const hash256& header_hash, uint64_t nonce)
{
    return hash_kernel(context, header_hash, nonce, calculate_dataset_item);
}

result hash(const epoch_context& context, const hash256& header_hash, uint64_t nonce)
{
    assert(context.full_dataset != nullptr);

    static constexpr auto lazy_lookup = [](const epoch_context& context, size_t index) {
        hash1024& item = context.full_dataset[index];
        if (item.words[0] == 0)
        {
            // TODO: Copy elision here makes it thread-safe?
            item = calculate_dataset_item(context, index);
        }

        return item;
    };

    return hash_kernel(context, header_hash, nonce, lazy_lookup);
}

bool verify(const epoch_context& context, const hash256& header_hash, const hash256& mix_hash,
    uint64_t nonce, uint64_t target)
{
    // TODO: Not optimal strategy.
    // First we should check if mix -> final transition is correct,
    // then check if the mix itself is valid.

    result r = hash_light(context, header_hash, nonce);

    if (std::memcmp(&r.mix_hash, &mix_hash, sizeof(mix_hash)) != 0)
        return false;

    // The final hash in BE order. Covert the top word to native integer.
    return from_be(r.final_hash.words[0]) < target;
}

uint64_t search_light(const epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations)
{
    const uint64_t end_nonce = start_nonce + iterations;
    for (uint64_t nonce = start_nonce; nonce < end_nonce; ++nonce)
    {
        result r = hash_light(context, header_hash, nonce);
        // FIXME: Must be converted from BE, not from LE.
        if (fix_endianness(r.final_hash.words[0]) < target)
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
        result r = hash(context, header_hash, nonce);
        // FIXME: Must be converted from BE, not from LE.
        if (fix_endianness(r.final_hash.words[0]) < target)
            return nonce;
    }
    return 0;
}

const char* version() noexcept
{
    return ethash_get_buildinfo()->project_version;
}
}
