/* Copyright 2018 Pawel Bylica.
   Licensed under the Apache License, Version 2.0. See the LICENSE file. */

#pragma once

#ifdef __cplusplus
#define NOEXCEPT noexcept

extern "C" {
#endif

struct ethash_epoch_context;

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


ethash_epoch_context* ethash_create_epoch_context(int epoch_number) NOEXCEPT;

void ethash_destroy_epoch_context(ethash_epoch_context* context) NOEXCEPT;

#ifdef __cplusplus
}
#endif
