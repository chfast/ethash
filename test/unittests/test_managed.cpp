// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "helpers.hpp"
#include "test_cases.hpp"

#include <ethash/ethash.hpp>

#include <gtest/gtest.h>

#include <array>
#include <future>

using namespace ethash;

TEST(managed, verify)
{
    for (const auto& t : hash_test_cases)
    {
        const hash256 header_hash = to_hash256(t.header_hash_hex);
        const hash256 mix_hash = to_hash256(t.mix_hash_hex);
        const hash256 final_hash = to_hash256(t.final_hash_hex);
        const uint64_t nonce = std::stoul(t.nonce_hex, nullptr, 16);
        const uint64_t target = final_hash.words[0] + 1;
        const bool valid = managed::verify(t.block_number, header_hash, mix_hash, nonce, target);
        EXPECT_TRUE(valid);
    }
}

TEST(managed_multithreaded, verify_all)
{
    constexpr size_t num_treads = 8;

    std::array<std::future<void>, num_treads> futures;
    for (auto& f : futures)
    {
        f = std::async(std::launch::async, [] {
            for (const auto& t : hash_test_cases)
            {
                const hash256 header_hash = to_hash256(t.header_hash_hex);
                const hash256 mix_hash = to_hash256(t.mix_hash_hex);
                const hash256 final_hash = to_hash256(t.final_hash_hex);
                const uint64_t nonce = std::stoul(t.nonce_hex, nullptr, 16);
                const uint64_t target = final_hash.words[0] + 1;
                const bool valid =
                    managed::verify(t.block_number, header_hash, mix_hash, nonce, target);
                EXPECT_TRUE(valid);
            }
        });
    }
    for (auto& f : futures)
        f.wait();
}

TEST(managed_multithreaded, verify_parallel)
{
    std::vector<std::future<bool>> futures;

    for (const auto& t : hash_test_cases)
    {
        futures.emplace_back(std::async(std::launch::async, [&t] {
            const hash256 header_hash = to_hash256(t.header_hash_hex);
            const hash256 mix_hash = to_hash256(t.mix_hash_hex);
            const hash256 final_hash = to_hash256(t.final_hash_hex);
            const uint64_t nonce = std::stoul(t.nonce_hex, nullptr, 16);
            const uint64_t target = final_hash.words[0] + 1;
            return managed::verify(t.block_number, header_hash, mix_hash, nonce, target);
        }));
    }

    for (auto& f : futures)
        EXPECT_TRUE(f.get());
}
