// Copyright 2018 Pawel Bylica.
// Governed by the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash-internal.hpp>
#include <ethash/keccak.hpp>

#include <benchmark/benchmark.h>


static void hash256(benchmark::State& state)
{
    const auto data_size = static_cast<size_t>(state.range(0));
    std::vector<uint64_t> data(data_size / sizeof(uint64_t), 0xdeadbeef);

    for (auto _ : state)
    {
        auto hash = ethash::keccak<256>(data.data(), data.size());
        benchmark::DoNotOptimize(hash.bytes);
    }
}
BENCHMARK(hash256)->Arg(0)->Arg(32)->Arg(40)->Arg(64)->Arg(96)->Arg(128);


static void hash512(benchmark::State& state)
{
    const auto data_size = static_cast<size_t>(state.range(0));
    std::vector<uint64_t> data(data_size / sizeof(uint64_t), 0xdeadbeef);

    for (auto _ : state)
    {
        auto hash = ethash::keccak<512>(data.data(), data.size());
        benchmark::DoNotOptimize(hash.bytes);
    }
}
BENCHMARK(hash512)->Arg(0)->Arg(32)->Arg(40)->Arg(64)->Arg(96)->Arg(128);


static void hash512_64(benchmark::State& state)
{
    std::vector<uint64_t> data(8, 0xabcdef);

    for (auto _ : state)
    {
        auto hash = ethash::keccak<512>(data.data(), data.size());
        benchmark::DoNotOptimize(hash.bytes);
    }
}
BENCHMARK(hash512_64);


static void double_hash_naive(benchmark::State& state)
{
    ethash::hash1024 hash;
    benchmark::ClobberMemory();

    for (auto _ : state)
    {
        hash.hashes[0] = ethash::keccak512(hash.hashes[0]);
        hash.hashes[1] = ethash::keccak512(hash.hashes[1]);
        benchmark::DoNotOptimize(hash.bytes);
    }
}
BENCHMARK(double_hash_naive);


static void double_hash_optimized(benchmark::State& state)
{
    ethash::hash1024 hash;
    benchmark::ClobberMemory();

    for (auto _ : state)
    {
        hash = ethash::double_keccak(hash);
        benchmark::DoNotOptimize(hash.bytes);
    }
}
BENCHMARK(double_hash_optimized);


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


static void calculate_dataset_item(benchmark::State& state)
{
    static ethash::epoch_context ctx = ethash::create_epoch_context(0);

    for (auto _ : state)
    {
        auto item = ethash::calculate_dataset_item(ctx, 1234);
        benchmark::DoNotOptimize(item.bytes);
    }
}
BENCHMARK(calculate_dataset_item);

BENCHMARK_MAIN();
