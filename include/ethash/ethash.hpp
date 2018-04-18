// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// @file
///
/// API design decisions:
///
/// 1. Signed integer type is used whenever the size of the type is not
///    restricted by the Ethash specification.
///    See http://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf.
///    See https://stackoverflow.com/questions/10168079/why-is-size-t-unsigned/.
///    See https://github.com/Microsoft/GSL/issues/171.

#pragma once

#include <ethash/ethash.h>

#include <cstdint>
#include <cstring>
#include <memory>

namespace ethash
{
static constexpr int epoch_length = ETHASH_EPOCH_LENGTH;
static constexpr int light_cache_item_size = ETHASH_LIGHT_CACHE_ITEM_SIZE;
static constexpr int full_dataset_item_size = ETHASH_FULL_DATASET_ITEM_SIZE;


union hash256
{
    uint64_t words[4] = {0};
    uint32_t hwords[8];
    char bytes[32];

    constexpr hash256() noexcept : words{0} {}

    /// Named constructor from bytes.
    ///
    /// @param bytes  A pointer to array of at least 32 bytes.
    /// @return       The constructed hash.
    static hash256 from_bytes(const uint8_t bytes[32]) noexcept
    {
        hash256 h;
        std::memcpy(&h, bytes, sizeof(h));
        return h;
    }
};

struct result
{
    hash256 final_hash;
    hash256 mix_hash;
};


/// Alias for ethash_calculate_light_cache_num_items().
static constexpr auto calculate_light_cache_num_items = ethash_calculate_light_cache_num_items;

/// Alias for ethash_calculate_full_dataset_num_items().
static constexpr auto calculate_full_dataset_num_items = ethash_calculate_full_dataset_num_items;


/// Calculates the epoch number out of the block number.
inline int get_epoch_number(int block_number)
{
    return block_number / epoch_length;
}

/**
 * Coverts the number of items of a light cache to size in bytes.
 *
 * @param num_items  The number of items in the light cache.
 * @return           The size of the light cache in bytes.
 */
inline size_t get_light_cache_size(int num_items) noexcept
{
    return static_cast<size_t>(num_items) * light_cache_item_size;
}

/**
 * Coverts the number of items of a full dataset to size in bytes.
 *
 * @param num_items  The number of items in the full dataset.
 * @return           The size of the full dataset in bytes.
 */
inline uint64_t get_full_dataset_size(int num_items) noexcept
{
    return static_cast<uint64_t>(num_items) * full_dataset_item_size;
}

inline int get_light_cache_num_items(const ethash_epoch_context& context)
{
    return ethash_get_light_cache_num_items(&context);
}

inline int get_full_dataset_num_items(const ethash_epoch_context& context)
{
    return ethash_get_full_dataset_num_items(&context);
}

/// Init full dataset in the epoch context.
///
/// This allocates the memory for the full dataset and inits dataset items
/// marked as "not-generated".
///
/// @param context  The epoch context.
/// @return  Returns true if memory allocations succeeded, false otherwise.
bool init_full_dataset(ethash_epoch_context& context) noexcept;


result hash_light(const ethash_epoch_context& context, const hash256& header_hash, uint64_t nonce);

result hash(const ethash_epoch_context& context, const hash256& header_hash, uint64_t nonce);

bool verify(const ethash_epoch_context& context, const hash256& header_hash,
    const hash256& mix_hash, uint64_t nonce, uint64_t target);

uint64_t search_light(const ethash_epoch_context& context, const hash256& header_hash,
    uint64_t target, uint64_t start_nonce, size_t iterations);

uint64_t search(const ethash_epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);


/// Tries to find the epoch number matching the given seed hash.
///
/// Mining pool protocols (many variants of stratum and "getwork") send out
/// seed hash instead of epoch number to workers. This function tries to recover
/// the epoch number from this seed hash.
///
/// @param seed  Ethash seed hash.
/// @return      The epoch number or -1 if not found.
int find_epoch_number(const hash256& seed) noexcept;


class epoch_context
{
public:
    explicit epoch_context(int epoch_number) noexcept
      : m_ctx{ethash_create_epoch_context(epoch_number)}
    {}

    ~epoch_context() noexcept
    {
        if (m_ctx)
            ethash_destroy_epoch_context(m_ctx);
    }

    epoch_context(const epoch_context&) = delete;
    epoch_context& operator=(const epoch_context&) = delete;

    epoch_context(epoch_context&& other) noexcept : m_ctx{other.m_ctx} { other.m_ctx = nullptr; }

    epoch_context& operator=(epoch_context&& other) noexcept
    {
        if (m_ctx)
            ethash_destroy_epoch_context(m_ctx);
        m_ctx = other.m_ctx;
        other.m_ctx = nullptr;
        return *this;
    }

    explicit operator bool() const noexcept { return m_ctx != nullptr; }

    ethash_epoch_context& operator*() noexcept { return *m_ctx; }

private:
    ethash_epoch_context* m_ctx = nullptr;
};


namespace managed
{
struct memory_ref
{
    const uint8_t* const data = nullptr;
    const size_t size = 0;

    memory_ref(const uint8_t* data, size_t size) : data{data}, size{size} {}
};

/// Get shared epoch context.
const ethash_epoch_context& get_epoch_context(int epoch_number);

memory_ref get_light_cache_data(int epoch_number);

/// Compute Ethash hash using light cache and the shared epoch context managed by the library.
result hash(int epoch_number, const hash256& header_hash, uint64_t nonce);

bool verify(int block_number, const hash256& header_hash, const hash256& mix_hash, uint64_t nonce,
    uint64_t target);
}
}
