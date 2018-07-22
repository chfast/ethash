// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "ethash-internal.hpp"

#include "endianness.hpp"
#include "primes.h"

#include <ethash/keccak.hpp>
#include <ethash/math_ops.hpp>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <limits>

#if __clang__
#define ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW \
    __attribute__((no_sanitize("unsigned-integer-overflow")))
#else
#define ATTRIBUTE_NO_SANITIZE_UNSIGNED_INTEGER_OVERFLOW
#endif

namespace ethash
{

#define PROGPOW_LANES                   32
#define PROGPOW_REGS                    16
#define PROGPOW_CACHE_BYTES             (16*1024)
#define PROGPOW_CNT_MEM                 ETHASH_NUM_DATASET_ACCESSES
#define PROGPOW_CNT_CACHE               8
#define PROGPOW_CNT_MATH                8
#define PROGPOW_CACHE_WORDS  (PROGPOW_CACHE_BYTES / sizeof(uint32_t))
#define PROGPOW_EPOCH_START (531)

// Helper to get the next value in the per-program random sequence
#define rnd()    (kiss99(&prog_rnd))
// Helper to pick a random mix location
#define mix_src() (rnd() % PROGPOW_REGS)
// Helper to access the sequence of mix destinations
#define mix_dst() (mix_seq[(mix_seq_cnt++)%PROGPOW_REGS])

// Internal constants:
constexpr static int light_cache_init_size = 1 << 24;
constexpr static int light_cache_growth = 1 << 17;
constexpr static int light_cache_rounds = 3;
constexpr static int full_dataset_init_size = 1 << 30;
constexpr static int full_dataset_growth = 1 << 23;
constexpr static int full_dataset_item_parents = 256;


// Verify constants:
static_assert(sizeof(hash512) == ETHASH_LIGHT_CACHE_ITEM_SIZE, "");
static_assert(sizeof(hash2048) == ETHASH_FULL_DATASET_ITEM_SIZE_256, "");
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

inline hash512 bitwise_xor(const hash512& x, const hash512& y) noexcept
{
    hash512 z;
    for (size_t i = 0; i < sizeof(z) / sizeof(z.words[0]); ++i)
        z.words[i] = x.words[i] ^ y.words[i];
    return z;
}

typedef struct {
	uint32_t z, w, jsr, jcong;
} kiss99_t;

// KISS99 is simple, fast, and passes the TestU01 suite
// https://en.wikipedia.org/wiki/KISS_(algorithm)
// http://www.cse.yorku.ca/~oz/marsaglia-rng.html
static
uint32_t kiss99(kiss99_t * st)
{
	uint32_t znew = (st->z = 36969 * (st->z & 65535) + (st->z >> 16));
	uint32_t wnew = (st->w = 18000 * (st->w & 65535) + (st->w >> 16));
	uint32_t MWC = ((znew << 16) + wnew);
	uint32_t SHR3 = (st->jsr ^= (st->jsr << 17), st->jsr ^= (st->jsr >> 13), st->jsr ^= (st->jsr << 5));
	uint32_t CONG = (st->jcong = 69069 * st->jcong + 1234567);
	return ((MWC^CONG) + SHR3);
}

static
uint32_t fnv1a(uint32_t *h, uint32_t d)
{
	return *h = (*h ^ d) * 0x1000193;
}

static
void fill_mix(
	uint64_t seed,
	uint32_t lane_id,
	uint32_t mix[PROGPOW_REGS]
)
{
	// Use FNV to expand the per-warp seed to per-lane
	// Use KISS to expand the per-lane seed to fill mix
	uint32_t fnv_hash = 0x811c9dc5;
	kiss99_t st;
	st.z = fnv1a(&fnv_hash, (uint32_t)seed);
	st.w = fnv1a(&fnv_hash, (uint32_t)(seed >> 32));
	st.jsr = fnv1a(&fnv_hash, lane_id);
	st.jcong = fnv1a(&fnv_hash, lane_id);
	for (int i = 0; i < PROGPOW_REGS; i++)
		mix[i] = kiss99(&st);
}

static
void swap(uint32_t *a, uint32_t *b)
{
	uint32_t t = *a;
	*a = *b;
	*b = t;
}

// Merge new data from b into the value in a
// Assuming A has high entropy only do ops that retain entropy
// even if B is low entropy
// (IE don't do A&B)
static
void merge(uint32_t *a, uint32_t b, uint32_t r)
{
	switch (r % 4)
	{
	case 0: *a = (*a * 33) + b; break;
	case 1: *a = (*a ^ b) * 33; break;
	case 2: *a = ROTL32(*a, ((r >> 16) % 32)) ^ b; break;
	case 3: *a = ROTR32(*a, ((r >> 16) % 32)) ^ b; break;
	}
}

// Random math between two input values
static
uint32_t math(uint32_t a, uint32_t b, uint32_t r)
{       
	switch (r % 11)
	{
	case 0: return a + b; break;
	case 1: return a * b; break;
	case 2: return mul_hi(a, b); break;
	case 3: return min_(a, b); break;
	case 4: return ROTL32(a, b); break;
	case 5: return ROTR32(a, b); break;
	case 6: return a & b; break;
	case 7: return a | b; break;
	case 8: return a ^ b; break;
	case 9: return clz(a) + clz(b); break;
	case 10: return popcount(a) + popcount(b); break;
	default: return 0;
	}
	return 0;
}

}  // namespace

int find_epoch_number(const hash256& seed) noexcept
{
    static constexpr int num_tries = 30000;  // Divisible by 16.

    // Thread-local cache of the last search.
    static thread_local int cached_epoch_number = 0;
    static thread_local hash256 cached_seed = {};

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

void build_light_cache(hash512* cache, int num_items, const hash256& seed) noexcept
{
    hash512 item = keccak512(seed.bytes, sizeof(seed));
    cache[0] = item;
    for (int i = 1; i < num_items; ++i)
    {
        item = keccak512(item);
        cache[i] = item;
    }

    for (int q = 0; q < light_cache_rounds; ++q)
    {
        for (int i = 0; i < num_items; ++i)
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


/// Calculates a full dataset item
///
/// This consist of two 512-bit items produced by calculate_dataset_item_partial().
/// Here the computation is done interleaved for better performance.
static hash512 calculate_dag_item(const epoch_context& context, int64_t index) noexcept
{
    const hash512* const cache = context.light_cache;

    static constexpr size_t num_half_words = sizeof(hash512) / sizeof(uint32_t);
    const int64_t num_cache_items = context.light_cache_num_items;

    uint32_t idx32 = static_cast<uint32_t>(index);

    hash512 mix0 = cache[index % num_cache_items];

    mix0.half_words[0] ^= fix_endianness(idx32);

    // Hash and convert to little-endian 32-bit words.
    mix0 = fix_endianness32(keccak512(mix0));

    for (uint32_t j = 0; j < full_dataset_item_parents; ++j)
    {
        uint32_t t0 = fnv(idx32 ^ j, mix0.half_words[j % num_half_words]);
        int64_t parent_index0 = t0 % num_cache_items;
        mix0 = fnv(mix0, fix_endianness32(cache[parent_index0]));
    }

    // Covert 32-bit words back to bytes and hash.
    mix0 = keccak512(fix_endianness32(mix0));

    return {mix0};
}

hash1024 calculate_dataset_item(const epoch_context& context, uint32_t index) noexcept
{
    hash512 n1,n2;
    int64_t idx64 = static_cast<int64_t>(index);
    n1 = calculate_dag_item(context, idx64*2);
    n2 = calculate_dag_item(context, idx64*2+1);
    return hash1024{{n1, n2}};    
}

/// Calculates a full dataset item for progpow
///
/// This consist of four 512-bit items produced by calculate_dataset_item_partial().
/// Here the computation is done interleaved for better performance.
hash2048 calculate_dataset_item_progpow(const epoch_context& context, uint32_t index) noexcept
{
    hash1024 n1,n2;
    n1 = calculate_dataset_item(context, index*2);
    n2 = calculate_dataset_item(context, index*2+1);
    return hash2048{{n1.hashes[0], n1.hashes[1], n2.hashes[0], n2.hashes[1]}};
}

/// Calculates a full l1 dataset item
///
/// This consist of one 32-bit items produced by calculate_dataset_item_partial().
uint32_t calculate_L1dataset_item(const epoch_context& context, uint32_t index) noexcept
{
    uint32_t idx = index/2;
    const hash2048 dag = calculate_dataset_item_progpow(context, (idx*2+101));
    uint64_t data = dag.words[0];
    uint32_t ret;
    ret = (uint32_t)((index%2)?(data>>32):(data));
    return ret;
}

namespace
{
using lookup_fn = hash1024 (*)(const epoch_context&, uint32_t);
using lookup_fn2 = hash2048 (*)(const epoch_context&, uint32_t);
using lookup_fn_l1 = uint32_t(*)(const epoch_context&, uint32_t);

inline hash512 hash_seed(const hash256& header_hash, uint64_t nonce) noexcept
{
    nonce = fix_endianness(nonce);
    uint8_t init_data[sizeof(header_hash) + sizeof(nonce)];
    std::memcpy(&init_data[0], &header_hash, sizeof(header_hash));
    std::memcpy(&init_data[sizeof(header_hash)], &nonce, sizeof(nonce));

    return keccak512(init_data, sizeof(init_data));
}

uint64_t keccak_f800(const ethash_hash256& header, const uint64_t seed, const uint32_t *result)
{
    uint32_t st[25];

    for (int i = 0; i < 25; i++)
        st[i] = 0;
    for (int i = 0; i < 8; i++)
        st[i] = header.hwords[i];
    st[8] = (uint32_t)seed;
    st[9] = (uint32_t)(seed >> 32);
    st[10] = result[0];
    st[11] = result[1];
    st[12] = result[2];
    st[13] = result[3];

    for (int r = 0; r < 21; r++) {
        keccak_f800_round(st, r);
    }
    // last round can be simplified due to partial output
    keccak_f800_round(st, 21);

    return (uint64_t)st[0] << 32 | st[1];//should be return (uint64_t)st[0] << 32 | st[1];
}

void keccak_f800(uint32_t* out, const ethash_hash256 header, const uint64_t seed, const uint32_t *result)
{
    uint32_t st[25];

    for (int i = 0; i < 25; i++)
        st[i] = 0;
    for (int i = 0; i < 8; i++)
        st[i] = header.hwords[i];
    st[8] = (uint32_t)seed;
    st[9] = (uint32_t)(seed >> 32);
    st[10] = result[0];
    st[11] = result[1];
    st[12] = result[2];
    st[13] = result[3];

    for (int r = 0; r < 21; r++) {
        keccak_f800_round(st, r);
    }
    // last round can be simplified due to partial output
    keccak_f800_round(st, 21);

    for (int i = 0; i < 8; ++i)
        out[i] = st[i];
}


hash256 ethash_keccak256(const ethash_hash256 header, const uint64_t seed, const uint32_t *result)
{
    hash256 hash;
    keccak_f800(hash.hwords, header, seed, result);
    return hash;
}

inline hash256 hash_final(const hash512& seed, const hash256& mix_hash)
{
    uint8_t final_data[sizeof(seed) + sizeof(mix_hash)];
    std::memcpy(&final_data[0], seed.bytes, sizeof(seed));
    std::memcpy(&final_data[sizeof(seed)], mix_hash.bytes, sizeof(mix_hash));
    return keccak256(final_data, sizeof(final_data));
}

static
void progPowInit(kiss99_t* prog_rnd, uint64_t prog_seed, uint32_t mix_seq[PROGPOW_REGS])
{
    uint32_t fnv_hash = 0x811c9dc5;
    prog_rnd->z = fnv1a(&fnv_hash, (uint32_t)prog_seed);
    prog_rnd->w = fnv1a(&fnv_hash, (uint32_t)(prog_seed >> 32));
    prog_rnd->jsr = fnv1a(&fnv_hash, (uint32_t)prog_seed);
    prog_rnd->jcong = fnv1a(&fnv_hash, (uint32_t)(prog_seed >> 32));
    // Create a random sequence of mix destinations for merge()
    // guaranteeing every location is touched once
    // Uses Fisher Yates shuffle
    for (uint32_t i = 0; i < PROGPOW_REGS; i++)
        mix_seq[i] = i;
    for (uint32_t i = PROGPOW_REGS - 1; i > 0; i--)
    {
        uint32_t j = kiss99(prog_rnd) % (i + 1);
        swap(&(mix_seq[i]), &(mix_seq[j]));
    }    
}

static
void progPowLoop(
    const epoch_context& context,
    const uint64_t prog_seed,
    const uint32_t loop,
    uint32_t mix[PROGPOW_LANES][PROGPOW_REGS],
    lookup_fn2  g_lut,
    lookup_fn_l1 c_lut)
{
    // All lanes share a base address for the global load
    // Global offset uses mix[0] to guarantee it depends on the load result
    uint32_t offset_g = mix[loop%PROGPOW_LANES][0] % (uint32_t)(context.full_dataset_num_items/2);

    hash2048 data256 = fix_endianness32(g_lut(context, offset_g));
    
    // Lanes can execute in parallel and will be convergent
    for (uint32_t l = 0; l < PROGPOW_LANES; l++)
    {
        // global load to sequential locations
        uint64_t data64 = data256.words[l];

        // initialize the seed and mix destination sequence
        uint32_t mix_seq[PROGPOW_REGS];
        int mix_seq_cnt = 0;
        kiss99_t prog_rnd;
        progPowInit(&prog_rnd, prog_seed, mix_seq);

        uint32_t offset, data32;
        //int max_i = max(PROGPOW_CNT_CACHE, PROGPOW_CNT_MATH);
        uint32_t max_i;
        if (PROGPOW_CNT_CACHE > PROGPOW_CNT_MATH)
            max_i = PROGPOW_CNT_CACHE;
        else
            max_i = PROGPOW_CNT_MATH;
        for (uint32_t i = 0; i < max_i; i++)
        {
            if (i < PROGPOW_CNT_CACHE)
            {
                // Cached memory access
                // lanes access random location
                offset = mix[l][mix_src()] % (uint32_t)PROGPOW_CACHE_WORDS;
                data32 = fix_endianness(c_lut(context, offset));
                merge(&(mix[l][mix_dst()]), data32, rnd());
            }
            if (i < PROGPOW_CNT_MATH)
            {
                // Random Math
                data32 = math(mix[l][mix_src()], mix[l][mix_src()], rnd());
                merge(&(mix[l][mix_dst()]), data32, rnd());
            }
        }
        // Consume the global load data at the very end of the loop
        // Allows full latency hiding
        merge(&(mix[l][0]), (uint32_t)data64, rnd());
        merge(&(mix[l][mix_dst()]), (uint32_t)(data64 >> 32), rnd());
    }
}

inline hash256 hash_kernel(
    const epoch_context& context, const hash512& seed, lookup_fn lookup) noexcept
{
    static constexpr size_t mix_hwords = sizeof(hash1024) / sizeof(uint32_t);
    const uint32_t index_limit = static_cast<uint32_t>(context.full_dataset_num_items);
    const uint32_t seed_init = fix_endianness(seed.half_words[0]);

    hash1024 mix{{fix_endianness32(seed), fix_endianness32(seed)}};

    for (uint32_t i = 0; i < num_dataset_accesses; ++i)
    {
        const uint32_t p = fnv(i ^ seed_init, mix.hwords[i % mix_hwords]) % index_limit;
        const hash1024 newdata = fix_endianness32(lookup(context, p));

        for (size_t j = 0; j < mix_hwords; ++j)
            mix.hwords[j] = fnv(mix.hwords[j], newdata.hwords[j]);
    }

    hash256 mix_hash;
    for (size_t i = 0; i < mix_hwords; i += 4)
    {
        const uint32_t h1 = fnv(mix.hwords[i], mix.hwords[i + 1]);
        const uint32_t h2 = fnv(h1, mix.hwords[i + 2]);
        const uint32_t h3 = fnv(h2, mix.hwords[i + 3]);
        mix_hash.hwords[i / 4] = h3;
    }

    return fix_endianness32(mix_hash);
}

hash256 progpow_kernel( const epoch_context& context, const uint64_t& seed,
  lookup_fn2 g_lut, lookup_fn_l1 c_lut) noexcept
{
    uint32_t mix[PROGPOW_LANES][PROGPOW_REGS];
    for(int i=0;i<PROGPOW_LANES;i++)for(int j=0;j<PROGPOW_REGS;j++)mix[i][j]=0;
    hash256 result;
    for (int i = 0; i < 8; i++)
        result.hwords[i] = 0;

    // initialize mix for all lanes
    for (uint32_t l = 0; l < PROGPOW_LANES; l++)
        fill_mix(seed, l, mix[l]);

    // execute the randomly generated inner loop
    for (uint32_t i = 0; i < PROGPOW_CNT_MEM; i++)
    {
        progPowLoop(context, (uint64_t)context.epoch_number, i, mix, g_lut, c_lut);
    }

    // Reduce mix data to a single per-lane result
    uint32_t lane_hash[PROGPOW_LANES];
    for (int l = 0; l < PROGPOW_LANES; l++)
    {
        lane_hash[l] = 0x811c9dc5;
        for (int i = 0; i < PROGPOW_REGS; i++)
            fnv1a(&lane_hash[l], mix[l][i]);
    }
    // Reduce all lanes to a single 128-bit result
    for (int i = 0; i < 8; i++)
        result.hwords[i] = 0x811c9dc5;
    for (int l = 0; l < PROGPOW_LANES; l++)
        fnv1a(&result.hwords[l % 8], lane_hash[l]);

    return fix_endianness32(result);
}
}  // namespace

result hash(const epoch_context& context, const hash256& header_hash, uint64_t nonce) noexcept
{
    const hash512 seed = hash_seed(header_hash, nonce);
    const hash256 mix_hash = hash_kernel(context, seed, calculate_dataset_item);
    return {hash_final(seed, mix_hash), mix_hash};
}

result hash(const epoch_context_full& context, const hash256& header_hash, uint64_t nonce) noexcept
{
    static const auto lazy_lookup = [](const epoch_context& context, uint32_t index) noexcept
    {
        auto full_dataset = static_cast<const epoch_context_full&>(context).full_dataset;
        hash1024& item = full_dataset[index];
        if (item.words[0] == 0)
        {
            // TODO: Copy elision here makes it thread-safe?
            item = calculate_dataset_item(context, index);
        }

        return item;
    };

    const hash512 seed = hash_seed(header_hash, nonce);
    const hash256 mix_hash = hash_kernel(context, seed, lazy_lookup);
    return {hash_final(seed, mix_hash), mix_hash};
}

result progpow(const epoch_context& context, const hash256& header_hash, uint64_t nonce) noexcept
{
    uint32_t result[4] = {0};

    uint64_t seed = keccak_f800(header_hash, nonce, result);

    const hash256 mix_hash = progpow_kernel(context, seed, calculate_dataset_item_progpow, calculate_L1dataset_item);

    return { ethash_keccak256(header_hash, seed, &mix_hash.hwords[0]), mix_hash};
}

result progpow(const epoch_context_full& context, const hash256& header_hash, uint64_t nonce) noexcept
{
    static const auto lazy_lookup = [](const epoch_context& context, uint32_t index) noexcept
    {
        auto full_dataset = static_cast<const epoch_context_full&>(context).full_dataset2;
        hash2048& item = full_dataset[index];
        if (item.words[0] == 0)
        {
            // TODO: Copy elision here makes it thread-safe?
            item = calculate_dataset_item_progpow(context, index);
        }

        return item;
    };

    static const auto lazy_l1_lookup = [](const epoch_context& context, uint32_t index) noexcept
    {
        auto full_l1_dataset = static_cast<const epoch_context_full&>(context).full_l1_dataset;
        uint32_t item = full_l1_dataset[index].hwords[0];
        if (item == 0)
        {
            // TODO: Copy elision here makes it thread-safe?
            item = calculate_L1dataset_item(context, index);
        }

        return item;
    };

    uint32_t result[4];
    for (int i = 0; i < 4; i++)
        result[i] = 0;
    uint64_t seed = keccak_f800(header_hash, nonce, result);

    const hash256 mix_hash = progpow_kernel(context, seed, lazy_lookup, lazy_l1_lookup);
    return { ethash_keccak256(header_hash, seed, &mix_hash.hwords[0]), mix_hash};
}

bool verify_final_hash(const hash256& header_hash, const hash256& mix_hash, uint64_t nonce,
    const hash256& boundary) noexcept
{
    const hash512 seed = hash_seed(header_hash, nonce);
    return is_less_or_equal(hash_final(seed, mix_hash), boundary);
}

bool verify_final_progpow(const hash256& header_hash, const hash256& mix_hash, uint64_t nonce,
    const hash256& boundary) noexcept
{
    uint32_t result[4];
    for (int i = 0; i < 4; i++)
        result[i] = 0;
    uint64_t seed = keccak_f800(header_hash, nonce, result);

    return is_less_or_equal(ethash_keccak256(header_hash, seed, &mix_hash.hwords[0]), boundary);
}

bool verify(const epoch_context& context, const hash256& header_hash, const hash256& mix_hash,
    uint64_t nonce, const hash256& boundary) noexcept
{
    const hash512 seed = hash_seed(header_hash, nonce);
    if (!is_less_or_equal(hash_final(seed, mix_hash), boundary))
        return false;

    const hash256 expected_mix_hash = hash_kernel(context, seed, calculate_dataset_item);
    return std::memcmp(expected_mix_hash.bytes, mix_hash.bytes, sizeof(mix_hash)) == 0;
}

bool verify_progpow(const epoch_context& context, const hash256& header_hash, const hash256& mix_hash,
    uint64_t nonce, const hash256& boundary) noexcept
{
    uint32_t result[4];
    for (int i = 0; i < 4; i++)
        result[i] = 0;
    uint64_t seed = keccak_f800(header_hash, nonce, result);

    if (!is_less_or_equal(ethash_keccak256(header_hash, seed, &mix_hash.hwords[0]), boundary))
        return false;

    const hash256 expected_mix_hash = progpow_kernel(context, seed, calculate_dataset_item_progpow, calculate_L1dataset_item);
    return std::memcmp(expected_mix_hash.bytes, mix_hash.bytes, sizeof(mix_hash)) == 0;
}

uint64_t search_light(const epoch_context& context, const hash256& header_hash,
    const hash256& boundary, uint64_t start_nonce, size_t iterations) noexcept
{
    const uint64_t end_nonce = start_nonce + iterations;
    for (uint64_t nonce = start_nonce; nonce < end_nonce; ++nonce)
    {
        result r = hash(context, header_hash, nonce);
        if (is_less_or_equal(r.final_hash, boundary))
            return nonce;
    }
    return 0;
}

uint64_t search(const epoch_context_full& context, const hash256& header_hash,
    const hash256& boundary, uint64_t start_nonce, size_t iterations) noexcept
{
    const uint64_t end_nonce = start_nonce + iterations;
    for (uint64_t nonce = start_nonce; nonce < end_nonce; ++nonce)
    {
        result r;
		if (context.epoch_number < PROGPOW_EPOCH_START) {
		    r = hash(context, header_hash, nonce);
		} else {
		    r = progpow(context, header_hash, nonce);
		}
        if (is_less_or_equal(r.final_hash, boundary))
            return nonce;
    }
    return 0;
}
}  // namespace ethash

using namespace ethash;

extern "C" {

ethash_hash256 ethash_calculate_epoch_seed(int epoch_number) noexcept
{
    ethash_hash256 epoch_seed = {};
    for (int i = 0; i < epoch_number; ++i)
        epoch_seed = ethash_keccak256_32(epoch_seed.bytes);
    return epoch_seed;
}

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
    int num_items = ethash_find_largest_prime(num_items_upper_bound);
    return num_items;
}

int ethash_calculate_full_dataset_num_items(int epoch_number) noexcept
{
    static int item_size = sizeof(hash2048); //TODO this is questionable, we should use the legacy notion
    if (epoch_number < PROGPOW_EPOCH_START) item_size = sizeof(hash1024);
    static int num_items_init = full_dataset_init_size / item_size;
    static int num_items_growth = full_dataset_growth / item_size;
    assert(full_dataset_init_size % item_size == 0);
    assert(full_dataset_growth % item_size == 0);

    int num_items_upper_bound = num_items_init + epoch_number * num_items_growth;
    int num_items = ethash_find_largest_prime(num_items_upper_bound);
    return num_items;
}

namespace
{
epoch_context_full* create_epoch_context(int epoch_number, bool full) noexcept
{
    static_assert(sizeof(epoch_context_full) < sizeof(hash512), "epoch_context too big");
    static constexpr size_t context_alloc_size = sizeof(hash512);

    const int light_cache_num_items = calculate_light_cache_num_items(epoch_number);
    const size_t light_cache_size = get_light_cache_size(light_cache_num_items);
    const size_t alloc_size = context_alloc_size + light_cache_size;

	if (alloc_size >= 131088776768) return nullptr;
    char* const alloc_data = static_cast<char*>(std::malloc(alloc_size));
    if (!alloc_data)
        return nullptr;  // Signal out-of-memory by returning null pointer.

    hash512* const light_cache = reinterpret_cast<hash512*>(alloc_data + context_alloc_size);
    const hash256 epoch_seed = calculate_epoch_seed(epoch_number);
    build_light_cache(light_cache, light_cache_num_items, epoch_seed);

    const int full_dataset_num_items = calculate_full_dataset_num_items(epoch_number);
    const int full_l1_dataset_num_items = 4*1024;
    hash2048* full_dataset = nullptr;
    hash32* full_l1_dataset = nullptr;
    if (full)
    {
        // TODO: This can be "optimized" by doing single allocation for light and full caches.
        const size_t num_items = static_cast<size_t>(full_dataset_num_items);
        const size_t num_l1_items = static_cast<size_t>(full_l1_dataset_num_items);
      if (epoch_number < PROGPOW_EPOCH_START) {
        full_dataset = static_cast<hash2048*>(std::calloc(num_items, sizeof(hash1024)));
      } else {
        full_dataset = static_cast<hash2048*>(std::calloc(num_items, sizeof(hash2048)));
        full_l1_dataset = static_cast<hash32*>(std::calloc(num_l1_items, sizeof(hash32)));
      }
        if (!full_dataset)
        {
            std::free(alloc_data);
            return nullptr;
        }
    }

    epoch_context_full* const context = new (alloc_data) epoch_context_full{
        epoch_number,
        light_cache_num_items,
        light_cache,
        full_dataset_num_items,
        full_dataset,
        full_l1_dataset,

    };
    return context;
}
}  // namespace

epoch_context* ethash_create_epoch_context(int epoch_number) noexcept
{
    return create_epoch_context(epoch_number, false);
}

epoch_context_full* ethash_create_epoch_context_full(int epoch_number) noexcept
{
    return create_epoch_context(epoch_number, true);
}

void ethash_destroy_epoch_context_full(epoch_context_full* context) noexcept
{
    std::free(context->full_dataset);
    ethash_destroy_epoch_context(context);
}

void ethash_destroy_epoch_context(epoch_context* context) noexcept
{
    context->~epoch_context();
    std::free(context);
}

void test_progpow_init2(uint8_t out[16], uint64_t seed, uint32_t m[16]) {
    progPowInit((kiss99_t*)out, seed, m);
}

}  // extern "C"
