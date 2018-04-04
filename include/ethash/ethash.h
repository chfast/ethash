/* Copyright 2018 Pawel Bylica.
   Licensed under the Apache License, Version 2.0. See the LICENSE file. */

#pragma once

#ifdef __cplusplus
#define NOEXCEPT noexcept

extern "C" {
#endif

struct ethash_epoch_context;

ethash_epoch_context* ethash_create_epoch_context(int epoch_number) NOEXCEPT;

void ethash_destroy_epoch_context(ethash_epoch_context* context) NOEXCEPT;

#ifdef __cplusplus
}
#endif
