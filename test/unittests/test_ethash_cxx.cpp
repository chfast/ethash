// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma GCC diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wpedantic"

#include <ethash/ethash.hpp>

#include <gtest/gtest.h>

using namespace ethash;

epoch_context invalid_global_context = [] {
    epoch_context c{0};
    epoch_context d{std::move(c)};
    return c;
}();

TEST(ethash_cxx, epoch_context_move)
{
    epoch_context c1{1};
    EXPECT_TRUE(c1);

    auto ptr = &*c1;

    epoch_context c2 = std::move(c1);
    EXPECT_TRUE(c2);
    EXPECT_FALSE(c1);
    EXPECT_EQ(&*c2, ptr);

    c1 = std::move(c2);
    EXPECT_TRUE(c1);
    EXPECT_FALSE(c2);

    epoch_context c3 = std::move(c2);
    EXPECT_FALSE(c3);
    EXPECT_FALSE(c2);
}

TEST(ethash_cxx, epoch_context_destructor)
{
    auto c = std::move(invalid_global_context);
}
