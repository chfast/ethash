// Copyright 2018 Pawel Bylica.
// Governed by the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash-internal.hpp>

#include <benchmark/benchmark.h>


static void seed(benchmark::State& state)
{
    const auto epoch_number = static_cast<uint32_t>(state.range(0));
    
    for (auto _ : state)
    {
        auto seed = ethash::calculate_seed(epoch_number);
        benchmark::DoNotOptimize(seed.bytes);
    }
}
BENCHMARK(seed)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);

static void light_cache(benchmark::State& state)
{
    const auto epoch_number = static_cast<uint32_t>(state.range(0));
    const auto size = ethash::calculate_light_cache_size(epoch_number);
    const auto seed = ethash::calculate_seed(epoch_number);

    for (auto _ : state)
    {
        auto cache = ethash::make_light_cache(size, seed);
        benchmark::DoNotOptimize(cache.data());
    }
}
BENCHMARK(light_cache)->Arg(1);

BENCHMARK_MAIN();
