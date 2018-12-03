// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/endianness.hpp>
#include <ethash/progpow-internal.hpp>
#include <ethash/progpow.hpp>

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
    const auto h0 = progpow::keccak_progpow_64({}, 0);
    const auto fh = progpow::keccak_progpow_256({}, 0, {});
    EXPECT_EQ(h0, 0x5dd431e5fbc604f4);
    EXPECT_EQ(to_hex(fh), "5dd431e5fbc604f499bfa0232f45f8f142d0ff5178f539e5a7800bf0643697af");

    const ethash::hash256 header_hash_2 =
        to_hash256("bc544c2baba832600013bd5d1983f592e9557d04b0fb5ef7a100434a5fc8d52a");
    const auto h2 = progpow::keccak_progpow_64(header_hash_2, 0x1ffffffff);
    const auto fh2 = progpow::keccak_progpow_256(header_hash_2, 0x1ffffffff, {});
    EXPECT_EQ(to_hex(fh2), "7be8a749a11cbc8eaacb7bd5b20ef17cac6545af701c91b8ee96b05226981ff4");
    EXPECT_EQ(h2, 0x7be8a749a11cbc8e);
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
    uint64_t s = keccak_progpow_64(header_hash, nonce);
    EXPECT_EQ(s, 0x00000002e29488b3);
    EXPECT_LT(s, boundary_prefix);
}

TEST(progpow, mix_rng_state)
{
    const int block_number = 30000;
    const uint64_t period_seed = block_number / progpow::period_length;

    progpow::mix_rng_state state{period_seed};

    EXPECT_EQ(state.rng(), 3572782357);
    EXPECT_EQ(state.rng(), 1081751690);

    using seq = std::array<uint32_t, progpow::num_regs>;

    const seq expected_dst_seq{{0, 4, 27, 26, 13, 15, 17, 7, 14, 8, 9, 12, 3, 10, 1, 11, 6, 16, 28,
        31, 2, 19, 30, 22, 29, 5, 24, 18, 25, 23, 21, 20}};
    seq dst_seq;
    std::generate(dst_seq.begin(), dst_seq.end(), [&] { return state.next_dst(); });
    EXPECT_EQ(dst_seq, expected_dst_seq);

    const seq expected_src_seq{{26, 30, 1, 19, 11, 21, 15, 18, 3, 17, 31, 16, 28, 4, 22, 23, 2, 13,
        29, 24, 10, 12, 5, 20, 7, 8, 14, 27, 6, 25, 9, 0}};
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

TEST(progpow, hash_empty)
{
    auto& context = get_ethash_epoch_context_0();

    const auto result = progpow::hash(context, 0, {}, 0);
    const auto mix_hex = "a09ffaa0f2b5d47a98c2d4fbc0e90936710dd2b2a220fce04e8d55a6c6a093d6";
    const auto final_hex = "7ea12cfc33f64616ab7dbbddf3362ee7dd3e1e20d60d860a85c51d6559c912c4";
    EXPECT_EQ(to_hex(result.mix_hash), mix_hex);
    EXPECT_EQ(to_hex(result.final_hash), final_hex);
}

TEST(progpow, hash_30000)
{
    const int block_number = 30000;
    const auto header =
        to_hash256("ffeeddccbbaa9988776655443322110000112233445566778899aabbccddeeff");
    const uint64_t nonce = 0x123456789abcdef0;

    auto context = ethash::create_epoch_context(ethash::get_epoch_number(block_number));

    const auto result = progpow::hash(*context, block_number, header, nonce);
    const auto mix_hex = "44fa88669c864aa30ba7da46e557593289c4d1fb143a1c43813d512b14fb4636";
    const auto final_hex = "b946ea7d74e3c619733ad73ac64a3c7671459b5d5d84d4f5c5cc09feb06ba2c3";
    EXPECT_EQ(to_hex(result.mix_hash), mix_hex);
    EXPECT_EQ(to_hex(result.final_hash), final_hex);
}

TEST(progpow, hash_and_verify)
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

        auto success = progpow::verify(
            *context, t.block_number, header_hash, result.mix_hash, nonce, result.final_hash);
        EXPECT_TRUE(success);

        auto lower_boundary = result.final_hash;
        --lower_boundary.bytes[31];
        auto final_failure = progpow::verify(
            *context, t.block_number, header_hash, result.mix_hash, nonce, lower_boundary);
        EXPECT_FALSE(final_failure);

        auto different_mix = result.mix_hash;
        ++different_mix.bytes[7];
        auto mix_failure = progpow::verify(
            *context, t.block_number, header_hash, different_mix, nonce, result.final_hash);
        EXPECT_FALSE(mix_failure);
    }
}

TEST(progpow, search)
{
    auto ctxp = ethash::create_epoch_context_full(0);
    auto& ctx = *ctxp;
    auto& ctxl = reinterpret_cast<const ethash::epoch_context&>(ctx);

    auto boundary = to_hash256("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
    auto sr = progpow::search(ctx, 0, {}, boundary, 0, 100);
    auto srl = progpow::search_light(ctxl, 0, {}, boundary, 0, 100);

    EXPECT_EQ(sr.mix_hash, ethash::hash256{});
    EXPECT_EQ(sr.final_hash, ethash::hash256{});
    EXPECT_EQ(sr.nonce, 0x0);
    EXPECT_EQ(sr.mix_hash, srl.mix_hash);
    EXPECT_EQ(sr.final_hash, srl.final_hash);
    EXPECT_EQ(sr.nonce, srl.nonce);

    sr = progpow::search(ctx, 0, {}, boundary, 100, 100);
    srl = progpow::search_light(ctxl, 0, {}, boundary, 100, 100);

    EXPECT_NE(sr.mix_hash, ethash::hash256{});
    EXPECT_NE(sr.final_hash, ethash::hash256{});
    EXPECT_EQ(sr.nonce, 103);
    EXPECT_EQ(sr.mix_hash, srl.mix_hash);
    EXPECT_EQ(sr.final_hash, srl.final_hash);
    EXPECT_EQ(sr.nonce, srl.nonce);

    auto r = progpow::hash(ctx, 0, {}, 103);
    EXPECT_EQ(sr.final_hash, r.final_hash);
    EXPECT_EQ(sr.mix_hash, r.mix_hash);
}

#if ETHASH_TEST_GENERATION
TEST(progpow, generate_hash_test_cases)
{
    constexpr auto num_epochs = 2;

    using namespace progpow;
    hash256 h{};
    for (int e = 0; e < num_epochs; ++e)
    {
        auto context = ethash::create_epoch_context(e);
        auto block_numbers = {
            e * epoch_length,
            e * epoch_length + period_length - 1,
            e * epoch_length + period_length,
            e * epoch_length + 2 * period_length - 1,
            (e + 1) * epoch_length - period_length,
            (e + 1) * epoch_length - 1,
        };
        for (auto b : block_numbers)
        {
            auto i = uint64_t(b);
            auto nonce = i * i * i * 977 + i * i * 997 + i * 1009;
            auto r = hash(*context, b, h, nonce);

            std::cout << "{" << b << ", \"" << to_hex(h) << "\", \"" << std::hex
                      << std::setfill('0') << std::setw(16) << nonce << std::dec << "\", \""
                      << to_hex(r.mix_hash) << "\", \"" << to_hex(r.final_hash) << "\"},\n";

            h = r.final_hash;
        }
    }
}
#endif