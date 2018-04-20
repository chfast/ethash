// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wpedantic"

#include <ethash/ethash.hpp>

#include <gtest/gtest.h>

using namespace ethash;

TEST(ethash_cxx, create_epoch_context)
{
    auto ctx1 = create_epoch_context(0);
    EXPECT_TRUE(ctx1);

    auto ptr = ctx1.get();

    auto ctx2 = std::move(ctx1);
    EXPECT_TRUE(ctx2);
    EXPECT_FALSE(ctx1);
    EXPECT_EQ(ctx2.get(), ptr);

    ctx1 = std::move(ctx2);
    EXPECT_TRUE(ctx1);
    EXPECT_FALSE(ctx2);

    const auto ctx3 = std::move(ctx2);
    EXPECT_FALSE(ctx3);
    EXPECT_FALSE(ctx2);

    EXPECT_TRUE(ctx1);
    ctx1 = create_epoch_context(0);
    EXPECT_TRUE(ctx1);
}
