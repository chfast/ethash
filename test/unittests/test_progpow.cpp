// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/progpow-internal.hpp>

#include "helpers.hpp"

#include <gtest/gtest.h>


TEST(progpow, keccak_progpow_64)
{
    ethash::hash256 extra{};
    const auto h0 = progpow::keccak_progpow_64({}, 0, {});
    const auto h1 = progpow::keccak_progpow_64({}, 0, extra);
    EXPECT_EQ(h0, h1);
    EXPECT_EQ(h0, 0xe531d45df404c6fb);

    const ethash::hash256 header_hash_2 =
        to_hash256("bc544c2baba832600013bd5d1983f592e9557d04b0fb5ef7a100434a5fc8d52a");
    for (uint32_t i = 0; i < 8; ++i)
        extra.hwords[i] = i + 1;
    const auto h2 = progpow::keccak_progpow_64(header_hash_2, 0x1ffffffff, extra);
    EXPECT_EQ(h2, 0xb5434c8218dd8826);
}

TEST(progpow, keccak_progpow_256)
{
    const auto h = progpow::keccak_progpow_256({}, 0, {});
    EXPECT_EQ(to_hex(h), "5dd431e5fbc604f499bfa0232f45f8f142d0ff5178f539e5a7800bf0643697af");
}

TEST(progpow, mix_rng_state)
{
    progpow::mix_rng_state state{0};
    EXPECT_EQ(state.rng(), 2062242187);
    EXPECT_EQ(state.rng(), 902361097);


    const std::array<uint32_t, 16> expected_sequance{
        {7, 12, 10, 5, 11, 4, 13, 6, 9, 1, 2, 15, 0, 8, 3, 14}};
    for (auto i : expected_sequance)
        EXPECT_EQ(state.next_index(), i);
}

TEST(progpow, random_math)
{
    const std::array<std::pair<uint32_t, uint32_t>, 22> inputs{{
        {20, 22},
        {70000, 80000},
        {70000, 80000},
        {1, 2},
        {3, 10000},
        {3, 0},
        {3, 6},
        {3, 6},
        {3, 6},
        {0, uint32_t(-1)},
        {3 << 13, 1 << 5},
        {22, 20},
        {80000, 70000},
        {80000, 70000},
        {2, 1},
        {10000, 3},
        {0, 3},
        {6, 3},
        {6, 3},
        {6, 3},
        {0, uint32_t(-1)},
        {3 << 13, 1 << 5},

    }};
    const std::array<uint32_t, 22> expected_results{{
        42,
        1305032704,
        1,
        1,
        196608,
        3,
        2,
        7,
        5,
        32,
        3,
        42,
        1305032704,
        1,
        1,
        80000,
        0,
        2,
        7,
        5,
        32,
        3,

    }};

    std::array<uint32_t, 22> results;
    for (uint32_t i = 0; i < results.size(); ++i)
        results[i] = progpow::random_math(inputs[i].first, inputs[i].second, i);
    EXPECT_EQ(results, expected_results);
}

TEST(progpow, random_merge)
{
    std::array<uint32_t, 8> input_a{
        {1000000, 2000000, 3000000, 4000000, 1000000, 2000000, 3000000, 4000000}};
    const std::array<uint32_t, 8> input_b{{101, 102, 103, 104, 0, 0, 0, 0}};

    for (uint32_t r = 0; r < input_a.size(); ++r)
        progpow::random_merge(input_a[r], input_b[r], r);

    const std::array<uint32_t, 8> expected_result{
        {33000101, 66003366, 2999975, 4000104, 33000000, 66000000, 3000000, 4000000}};
    EXPECT_EQ(input_a, expected_result);
}

TEST(progpow, l1_cache)
{
    auto& context = get_ethash_epoch_context_0();

    constexpr auto test_size = 20;
    std::array<uint32_t, test_size> cache_slice;
    for (size_t i = 0; i < cache_slice.size(); ++i)
        cache_slice[i] = context.l1_cache[i];

    const std::array<uint32_t, test_size> expected{
        {690150178, 1181503948, 2248155602, 2118233073, 2193871115, 1791778428, 1067701239,
            724807309, 530799275, 3480325829, 3899029234, 1998124059, 2541974622, 1100859971,
            1297211151, 3268320000, 2217813733, 2690422980, 3172863319, 2651064309}};
    EXPECT_EQ(cache_slice, expected);
}

TEST(progpow, hash_null_bytes)
{
    auto& context = get_ethash_epoch_context_0();

    auto r = progpow::hash(context, 0, {}, 0);
    auto expected_final_hash = "7d5b1d047bfb2ebeff3f60d6cc935fc1eb882ece1732eb4708425d2f11965535";
    auto expected_mix_hash = "8c091b4eebc51620ca41e2b90a167d378dbfe01c0a255f70ee7004d85a646e17";
    EXPECT_EQ(to_hex(r.final_hash), expected_final_hash);
    EXPECT_EQ(to_hex(r.mix_hash), expected_mix_hash);
}
