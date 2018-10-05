// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ubqhash.hpp>

#include "helpers.hpp"

#include <gtest/gtest.h>


TEST(ubqhash, epoch_context_0)
{
    auto context = ubqhash::create_epoch_context(0);
    EXPECT_EQ(context->epoch_number, 0);
    EXPECT_EQ(context->light_cache_num_items, 262139);
    EXPECT_EQ(context->full_dataset_num_items, 8388593);

    const auto light_cache_0_hex =
        "ecb42b0a0edaf1d5f23122eed10bff137a62d939396d62a157623402ee6ddd3508fad6f0c80864c03d10bf2d29"
        "c75b836e885d3fac052ba0926e5df71f16ca83";
    EXPECT_EQ(to_hex(context->light_cache[0]), light_cache_0_hex);
}
