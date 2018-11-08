// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/endianness.hpp>
#include <ethash/progpow-internal.hpp>

#include "helpers.hpp"
#include "progpow_test_vectors.hpp"

#include <gtest/gtest.h>


TEST(progpow, keccak_progpow_256)
{
    const auto h = progpow::keccak_progpow_256({}, 0, {});
    EXPECT_EQ(to_hex(h), "5dd431e5fbc604f499bfa0232f45f8f142d0ff5178f539e5a7800bf0643697af");
}

TEST(progpow, keccak_progpow_64)
{
    ethash::hash256 extra{};
    const auto h0 = progpow::keccak_progpow_64({}, 0, {});
    const auto h1 = progpow::keccak_progpow_64({}, 0, extra);
    const auto fh = progpow::keccak_progpow_256({}, 0, {});
    EXPECT_EQ(h0, h1);
    EXPECT_EQ(h0, 0x5dd431e5fbc604f4);
    EXPECT_EQ(to_hex(fh), "5dd431e5fbc604f499bfa0232f45f8f142d0ff5178f539e5a7800bf0643697af");

    const ethash::hash256 header_hash_2 =
        to_hash256("bc544c2baba832600013bd5d1983f592e9557d04b0fb5ef7a100434a5fc8d52a");
    for (uint32_t i = 0; i < 8; ++i)
        extra.word32s[i] = i + 1;
    const auto h2 = progpow::keccak_progpow_64(header_hash_2, 0x1ffffffff, extra);
    const auto fh2 = progpow::keccak_progpow_256(header_hash_2, 0x1ffffffff, extra);
    EXPECT_EQ(to_hex(fh2), "824c43b52688dd1857fc271d677e6f3a21906d8f8eb4bf68bb5b84d58e8e3433");
    EXPECT_EQ(h2, 0x824c43b52688dd18);
}

TEST(progpow, keccak_progpow_64boundary)
{
    using namespace progpow;

    const hash256 header_hash{};
    const uint64_t nonce = 254984491;
    const hash256 boundary =
        to_hash256("0000000f00000000000000000000000000000000000000000000000000000000");
    const uint64_t boundary_prefix = be::uint64(boundary.word64s[0]);

    hash256 h = keccak_progpow_256(header_hash, nonce, {});
    EXPECT_EQ(to_hex(h), "00000002e29488b39928408765b4645343fd9dd7ed2a4d4ddf42bf1c19ff8bac");

    // The full hash is smaller than the boundary hash:
    EXPECT_LT(std::memcmp(h.bytes, boundary.bytes, sizeof(h)), 0);

    // The approximated comparison can be done against 64-bit prefix.
    uint64_t s = keccak_progpow_64(header_hash, nonce, {});
    EXPECT_EQ(s, 0x00000002e29488b3);
    EXPECT_LT(s, boundary_prefix);
}

TEST(progpow, mix_rng_state)
{
    progpow::mix_rng_state state{0};
    EXPECT_EQ(state.rng(), 2821196981);
    EXPECT_EQ(state.rng(), 3728806798);

    using seq = std::array<uint32_t, progpow::num_regs>;

    const seq expected_dst_seq{{11, 0, 9, 10, 5, 8, 2, 13, 12, 7, 3, 6, 4, 15, 1, 14}};
    seq dst_seq;
    std::generate(dst_seq.begin(), dst_seq.end(), [&] { return state.next_dst(); });
    EXPECT_EQ(dst_seq, expected_dst_seq);

    const seq expected_src_seq{{9, 1, 0, 14, 13, 2, 12, 4, 7, 3, 8, 10, 11, 6, 15, 5}};
    seq src_seq;
    std::generate(src_seq.begin(), src_seq.end(), [&] { return state.next_src(); });
    EXPECT_EQ(src_seq, expected_src_seq);
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

TEST(progpow, DISABLED_hash)
{
    ethash::epoch_context_ptr context{nullptr, nullptr};

    for (auto& t : progpow_hash_test_cases)
    {
        const auto epoch_number = ethash::get_epoch_number(t.block_number);
        if (!context || context->epoch_number != epoch_number)
            context = ethash::create_epoch_context(epoch_number);

        const auto header_hash = to_hash256(t.header_hash_hex);
        const auto nonce = std::stoull(t.nonce_hex, nullptr, 16);
        const auto result = progpow::hash(*context, t.block_number, header_hash, nonce);
        EXPECT_EQ(to_hex(result.mix_hash), t.mix_hash_hex);
        EXPECT_EQ(to_hex(result.final_hash), t.final_hash_hex);
    }
}

#if ETHASH_TEST_GENERATION
TEST(progpow, generate_hash_test_cases)
{
    auto context = ethash::create_epoch_context(0);

    auto generate_test_case = [&context](uint64_t i) noexcept
    {
        auto n = static_cast<int>(i);
        auto e = ethash::get_epoch_number(n);
        if (context->epoch_number != e)
            context = ethash::create_epoch_context(e);

        uint64_t nonce = i * i * i * 977 + i * i * 997 + i * 1009;

        ethash::hash256 h{};
        if (i > 0)
        {
            size_t s = sizeof(h);
            size_t num_byte = s - (((i - 1) / 8) % s) - 1;
            size_t bit = (i - 1) % 8;
            h.bytes[num_byte] = uint8_t(1 << bit);
        }

        auto r = progpow::hash(*context, n, h, nonce);
        std::cout << "{" << i << ", \"" << to_hex(h) << "\", \"" << std::hex << std::setfill('0')
                  << std::setw(16) << nonce << std::dec << "\", \"" << to_hex(r.mix_hash)
                  << "\", \"" << to_hex(r.final_hash) << "\"},\n";
    };

    for (uint64_t i = 0; i < 257; ++i)
        generate_test_case(i);
    for (uint64_t i = 100000; i < 100257; ++i)
        generate_test_case(i);
}
#endif