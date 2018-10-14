// Ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "../unittests/helpers.hpp"

#include <ethash/progpow-internal.hpp>

#include <benchmark/benchmark.h>

static void progpow_build_l1_cache(benchmark::State& state)
{
    auto& context = get_ethash_epoch_context_0();
    std::array<uint32_t, progpow::l1_cache_num_items> l1_cache;

    for (auto _ : state)
    {
        progpow::build_l1_cache(l1_cache.data(), context);
        benchmark::DoNotOptimize(l1_cache.data());
    }
}
BENCHMARK(progpow_build_l1_cache)->Unit(benchmark::kMillisecond);

static void progpow_mix_rng(benchmark::State& state)
{
    progpow::mix_rng_state rng_state{0xff};
    benchmark::ClobberMemory();
    for (auto _ : state)
    {
        for (size_t x = 0; x < 16; ++x)
        {
            auto i = rng_state.next_index();
            benchmark::DoNotOptimize(i);
        }
    }
}
BENCHMARK(progpow_mix_rng);

static void progpow_hash(benchmark::State& state)
{
    // Get block number in millions.
    int block_number = static_cast<int>(state.range(0)) * 1000000;
    uint64_t nonce = 1;

    const auto& ctx = ethash::get_global_epoch_context(ethash::get_epoch_number(block_number));

    for (auto _ : state)
        progpow::hash(ctx, block_number++, {}, nonce++);
}
BENCHMARK(progpow_hash)->Unit(benchmark::kMicrosecond)->Arg(0)->Arg(10);
