// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "ethash-internal.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "keccak.hpp"
#include "params.hpp"
#include "utils.hpp"

#if defined(__has_attribute)
#if __has_attribute(no_sanitize)
#define ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW \
    __attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
#endif

#if !defined(ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW)
#define ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW
#endif

namespace ethash
{
// Verify constants:
static_assert(sizeof(hash512) == ETHASH_LIGHT_CACHE_ITEM_SIZE, "");
static_assert(sizeof(hash1024) == ETHASH_FULL_DATASET_ITEM_SIZE, "");
static_assert(light_cache_item_size == ETHASH_LIGHT_CACHE_ITEM_SIZE, "");
static_assert(full_dataset_item_size == ETHASH_FULL_DATASET_ITEM_SIZE, "");

namespace
{
ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW
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

hash256 calculate_seed(int epoch_number) noexcept
{
    hash256 seed;
    for (int i = 0; i < epoch_number; ++i)
        seed = keccak256(seed);
    return seed;
}

int find_epoch_number(const hash256& seed) noexcept
{
    static constexpr int num_tries = 30000;  // Divisible by 16.

    // Thread-local cache of the last search.
    static thread_local int cached_epoch_number = 0;
    static thread_local hash256 cached_seed;

    // Load from memory once (memory will be clobbered by keccak256()).
    uint32_t seed_part = seed.hwords[0];
    const int e = cached_epoch_number;
    hash256 s = cached_seed;

    if (s.hwords[0] == seed_part)
        return e;

    // Try the next seed, will match for sequential epoch access.
    s = keccak256(s);
    if (s.hwords[0] == seed_part)
    {
        cached_seed = s;
        cached_epoch_number = e + 1;
        return e + 1;
    }

    // Search for matching seed starting from epoch 0.
    s = {};
    for (int i = 0; i < num_tries; ++i)
    {
        if (s.hwords[0] == seed_part)
        {
            cached_seed = s;
            cached_epoch_number = i;
            return i;
        }

        s = keccak256(s);
    }

    return -1;
}

void build_light_cache(hash512* cache, int num_items, const hash256 &seed)
{
    hash512 item = keccak512(seed);
    cache[0] = item;
    for (int64_t i = 1; i < num_items; ++i)
    {
        item = keccak512(item);
        cache[i] = item;
    }

    for (int q = 0; q < light_cache_rounds; ++q)
    {
        for (int64_t i = 0; i < num_items; ++i)
        {
            const uint32_t index_limit = static_cast<uint32_t>(num_items);

            // Fist index: 4 first bytes of the item as little-endian integer.
            uint32_t t = fix_endianness(cache[i].half_words[0]);
            uint32_t v = t % index_limit;

            // Second index.
            uint32_t w = static_cast<uint32_t>(num_items + (i - 1)) % index_limit;

            // Pipelining functions returning structs gives small performance boost.
            cache[i] = keccak512(bitwise_xor(cache[v], cache[w]));
        }
    }
}

/// TODO: Only used in tests or for reference, so can be removed or moved.
hash512 calculate_dataset_item_partial(const hash512* cache, int num_cache_items, size_t index) noexcept
{
    // FIXME: Remove this function.
    static constexpr size_t num_half_words = sizeof(hash512) / sizeof(uint32_t);

    const size_t index_limit = static_cast<size_t>(num_cache_items);
    const uint32_t init = static_cast<uint32_t>(index);

    hash512 mix = cache[index % index_limit];
    mix.half_words[0] ^= fix_endianness(init);
    mix = keccak512(mix);
    mix = fix_endianness32(mix);  // Covert bytes to 32-bit words.

    for (uint32_t j = 0; j < full_dataset_item_parents; ++j)
    {
        uint32_t t = fnv(init ^ j, mix.half_words[j % num_half_words]);
        size_t parent_index = t % index_limit;
        mix = fnv(mix, fix_endianness32(cache[parent_index]));
    }

    return keccak512(fix_endianness32(mix));
}


/// Calculates a full dataset item
///
/// This consist of two 512-bit items produced by calculate_dataset_item_partial().
/// Here the computation is done interleaved for better performance.
hash1024 calculate_dataset_item(const ethash_epoch_context& context, size_t index) noexcept
{
    // FIXME: Consider changing the type of index.
    const hash512* cache = context.light_cache;

    static constexpr size_t num_half_words = sizeof(hash512) / sizeof(uint32_t);
    const int64_t num_cache_items = static_cast<int64_t>(context.light_cache_num_items);

    const int64_t index0 = int64_t(index) * 2;
    const int64_t index1 = int64_t(index) * 2 + 1;

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
        int64_t parent_index0 = t0 % num_cache_items;
        mix.hashes[0] = fnv(mix.hashes[0], fix_endianness32(cache[parent_index0]));

        uint32_t t1 = fnv(init1 ^ j, mix.hashes[1].half_words[j % num_half_words]);
        int64_t parent_index1 = t1 % num_cache_items;
        mix.hashes[1] = fnv(mix.hashes[1], fix_endianness32(cache[parent_index1]));
    }

    return double_keccak(fix_endianness32(mix));  // Covert 32-bit words back to bytes and hash.
}

bool init_full_dataset(ethash_epoch_context& context) noexcept
{
    assert(context.full_dataset == nullptr);

    const size_t num_items = static_cast<size_t>(context.full_dataset_num_items);
    context.full_dataset = static_cast<hash1024*>(std::calloc(num_items, sizeof(hash1024)));
    return context.full_dataset != nullptr;
}

namespace
{

using lookup_fn = hash1024 (*)(const ethash_epoch_context&, size_t);

inline result hash_kernel(const ethash_epoch_context& context, const hash256& header_hash,
    uint64_t nonce, lookup_fn lookup)
{
    static constexpr size_t mix_hwords = sizeof(hash1024) / sizeof(uint32_t);
    const int num_items = context.full_dataset_num_items;
    const uint32_t index_limit = static_cast<uint32_t>(num_items);

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
        auto p = fnv(i ^ s_init, mix.hwords[i % mix_hwords]) % index_limit;
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

result hash_light(const ethash_epoch_context& context, const hash256& header_hash, uint64_t nonce)
{
    return hash_kernel(context, header_hash, nonce, calculate_dataset_item);
}

result hash(const ethash_epoch_context& context, const hash256& header_hash, uint64_t nonce)
{
    assert(context.full_dataset != nullptr);

    static const auto lazy_lookup = [](const ethash_epoch_context& context, size_t index) {
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

bool verify(const ethash_epoch_context& context, const hash256& header_hash, const hash256& mix_hash,
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

uint64_t search_light(const ethash_epoch_context& context, const hash256& header_hash,
    uint64_t target, uint64_t start_nonce, size_t iterations)
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

uint64_t search(const ethash_epoch_context& context, const hash256& header_hash, uint64_t target,
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
}  // namespace ethash

using namespace ethash;

extern "C" {

int ethash_calculate_light_cache_num_items(int epoch_number) noexcept
{
    static constexpr int item_size = sizeof(hash512);
    static constexpr int num_items_init = light_cache_init_size / item_size;
    static constexpr int num_items_growth = light_cache_growth / item_size;
    static_assert(
        light_cache_init_size % item_size == 0, "light_cache_init_size not multiple of item size");
    static_assert(
        light_cache_growth % item_size == 0, "light_cache_growth not multiple of item size");

    int num_items_upper_bound = num_items_init + epoch_number * num_items_growth;
    int num_items = find_largest_prime(num_items_upper_bound);
    return num_items;
}

int ethash_calculate_full_dataset_num_items(int epoch_number) noexcept
{
    static constexpr int item_size = sizeof(hash1024);
    static constexpr int num_items_init = full_dataset_init_size / item_size;
    static constexpr int num_items_growth = full_dataset_growth / item_size;
    static_assert(full_dataset_init_size % item_size == 0,
                  "full_dataset_init_size not multiple of item size");
    static_assert(
        full_dataset_growth % item_size == 0, "full_dataset_growth not multiple of item size");

    int num_items_upper_bound = num_items_init + epoch_number * num_items_growth;
    int num_items = find_largest_prime(num_items_upper_bound);
    return num_items;
}

ethash_epoch_context* ethash_create_epoch_context(int epoch_number) noexcept
{
    static_assert(sizeof(ethash_epoch_context) < sizeof(hash512), "ethash_epoch_context too big");
    static constexpr size_t context_alloc_size = sizeof(hash512);

    const int light_cache_num_items = calculate_light_cache_num_items(epoch_number);
    const size_t light_cache_size = get_light_cache_size(light_cache_num_items);
    const size_t alloc_size = context_alloc_size + light_cache_size;

    char* const alloc_data = static_cast<char*>(std::malloc(alloc_size));
    if (!alloc_data)
        return nullptr;  // Signal out-of-memory by returning null pointer.

    hash512* const light_cache = reinterpret_cast<hash512*>(alloc_data + context_alloc_size);
    const hash256 seed = calculate_seed(epoch_number);
    build_light_cache(light_cache, light_cache_num_items, seed);

    ethash_epoch_context* const context = new (alloc_data) ethash_epoch_context;
    context->epoch_number = epoch_number;
    context->light_cache_num_items = light_cache_num_items;
    context->light_cache = light_cache;
    context->full_dataset_num_items = calculate_full_dataset_num_items(epoch_number);
    return context;
}

void ethash_destroy_epoch_context(ethash_epoch_context* context) noexcept
{
    std::free(context->full_dataset);
    context->~ethash_epoch_context();
    std::free(context);
}

int ethash_get_light_cache_num_items(const ethash_epoch_context* context) noexcept
{
    return context->light_cache_num_items;
}

int ethash_get_full_dataset_num_items(const ethash_epoch_context* context) noexcept
{
    return context->full_dataset_num_items;
}

}  // extern "C"
