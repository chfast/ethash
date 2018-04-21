/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ETHASH_EPOCH_LENGTH 30000
#define ETHASH_LIGHT_CACHE_ITEM_SIZE 64
#define ETHASH_FULL_DATASET_ITEM_SIZE 128


union ethash_hash256
{
    uint64_t words[4];
    uint32_t hwords[8];
    uint8_t bytes[32];
};

union ethash_hash512
{
    uint64_t words[8];
    uint32_t half_words[16];
    char bytes[64];
};

union ethash_hash1024
{
    /* TODO: Is the array worse than 2 fields for memory aliasing? */
    union ethash_hash512 hashes[2];
    uint64_t words[16];
    uint32_t hwords[32];
    uint8_t bytes[128];
};


struct ethash_epoch_context
{
    const int epoch_number;
    const int light_cache_num_items;
    const union ethash_hash512* light_cache;
    const int full_dataset_num_items;
    union ethash_hash1024* full_dataset;
};


/**
 * Calculates the number of items in the light cache for given epoch.
 *
 * This function will search for a prime number matching the criteria given
 * by the Ethash so the execution time is not constant. It takes ~ 0.01 ms.
 *
 * @param epoch_number  The epoch number.
 * @return              The number items in the light cache.
 */
int ethash_calculate_light_cache_num_items(int epoch_number) NOEXCEPT;


/**
 * Calculates the number of items in the full dataset for given epoch.
 *
 * This function will search for a prime number matching the criteria given
 * by the Ethash so the execution time is not constant. It takes ~ 0.05 ms.
 *
 * @param epoch_number  The epoch number.
 * @return              The number items in the full dataset.
 */
int ethash_calculate_full_dataset_num_items(int epoch_number) NOEXCEPT;


struct ethash_epoch_context* ethash_create_epoch_context(int epoch_number) NOEXCEPT;

void ethash_destroy_epoch_context(struct ethash_epoch_context* context) NOEXCEPT;

#ifdef __cplusplus
}
#endif
