// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/blake2b.h>

#include "helpers.hpp"

#include <gtest/gtest.h>

#include <cstring>


TEST(blake2b, empty_input)
{
    static constexpr char expected_hex[] =
        "786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee58531389644493"
        "4eb04b903a685b1448b755d56f701afe9be2ce";
    auto h = ethash_blake2b_512(nullptr, 0);
    EXPECT_EQ(to_hex(h), expected_hex);
}

TEST(blake2b, known_input)
{
    static constexpr char input[] = "The quick brown fox jumps over the lazy dog";
    static constexpr char expected_hex[] =
        "a8add4bdddfd93e4877d2746e62817b116364a1fa7bc148d95090bc7333b3673f82401cf7aa2e4cb1ecd90296e"
        "3f14cb5413f8ed77be73045b13914cdcd6a918";
    auto h = ethash_blake2b_512(reinterpret_cast<const uint8_t*>(input), std::strlen(input));
    EXPECT_EQ(to_hex(h), expected_hex);
}
