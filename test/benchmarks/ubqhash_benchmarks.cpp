// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ubqhash-internal.hpp>

#include <benchmark/benchmark.h>


static void ubqhash_light_cache(benchmark::State& state)
{
    const int epoch_number = static_cast<int>(state.range(0));
    const auto num_items = ethash::calculate_light_cache_num_items(epoch_number);
    const auto seed = ethash::calculate_epoch_seed(epoch_number);

    std::unique_ptr<ethash::hash512[]> light_cache{new ethash::hash512[num_items]};

    for (auto _ : state)
    {
        ubqhash::build_light_cache(light_cache.get(), num_items, seed);
        benchmark::DoNotOptimize(light_cache.get());
    }
}
BENCHMARK(ubqhash_light_cache)->Arg(1)->Unit(benchmark::kMillisecond);
