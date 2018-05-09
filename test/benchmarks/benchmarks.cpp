// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "../unittests/helpers.hpp"

#include <ethash/ethash-internal.hpp>
#include <ethash/keccak.hpp>
#include <ethash/primes.hpp>

#include <benchmark/benchmark.h>


static void is_prime(benchmark::State& state)
{
    const auto n = static_cast<int>(state.range(0));

    for (auto _ : state)
    {
        auto answer = ethash::is_prime(n);
        benchmark::DoNotOptimize(&answer);
    }
}
BENCHMARK(is_prime)->Arg(2147483647)->Arg(2147483645)->Arg(2147483631)->Arg(2147483629);

static void calculate_light_cache_num_items(benchmark::State& state)
{
    const auto epoch_number = static_cast<int>(state.range(0));

    for (auto _ : state)
    {
        auto answer = ethash::calculate_light_cache_num_items(epoch_number);
        benchmark::DoNotOptimize(&answer);
    }
}
BENCHMARK(calculate_light_cache_num_items)->Arg(32638)->Arg(32639);

static void calculate_full_dataset_num_items(benchmark::State& state)
{
    const auto epoch_number = static_cast<int>(state.range(0));

    for (auto _ : state)
    {
        auto answer = ethash::calculate_full_dataset_num_items(epoch_number);
        benchmark::DoNotOptimize(&answer);
    }
}
BENCHMARK(calculate_full_dataset_num_items)->Arg(32638)->Arg(32639);


static void seed(benchmark::State& state)
{
    const int epoch_number = static_cast<int>(state.range(0));

    for (auto _ : state)
    {
        auto seed = ethash::calculate_seed(epoch_number);
        benchmark::DoNotOptimize(seed.bytes);
    }
}
BENCHMARK(seed)->Arg(1)->Arg(10)->Arg(100)->Arg(1000)->Arg(10000);


static void light_cache(benchmark::State& state)
{
    const int epoch_number = static_cast<int>(state.range(0));
    const auto num_items = ethash::calculate_light_cache_num_items(epoch_number);
    const auto seed = ethash::calculate_seed(epoch_number);

    std::unique_ptr<ethash::hash512[]> light_cache{new ethash::hash512[num_items]};

    for (auto _ : state)
    {
        ethash::build_light_cache(light_cache.get(), num_items, seed);
        benchmark::DoNotOptimize(light_cache.get());
    }
}
BENCHMARK(light_cache)->Arg(1);


static void calculate_dataset_item(benchmark::State& state)
{
    static auto ctx = ethash::create_epoch_context(0);

    for (auto _ : state)
    {
        auto item = ethash::calculate_dataset_item(*ctx, 1234);
        benchmark::DoNotOptimize(item.bytes);
    }
}
BENCHMARK(calculate_dataset_item);


static void verify(benchmark::State& state)
{
    const int block_number = 5000000;
    const ethash::hash256 header_hash =
        to_hash256("bc544c2baba832600013bd5d1983f592e9557d04b0fb5ef7a100434a5fc8d52a");
    const ethash::hash256 mix_hash =
        to_hash256("94cd4e844619ee20989578276a0a9046877d569d37ba076bf2e8e34f76189dea");
    const uint64_t nonce = 0x4617a20003ba3f25;
    const uint64_t target = 0x0000000000001a5b + 1;

    static const auto ctx = ethash::create_epoch_context(ethash::get_epoch_number(block_number));

    for (auto _ : state)
        ethash::verify(*ctx, header_hash, mix_hash, nonce, target);
}
BENCHMARK(verify)->Threads(1)->Threads(2)->Threads(4)->Threads(8);


static void verify_managed(benchmark::State& state)
{
    const int block_number = 5000000;
    const ethash::hash256 header_hash =
        to_hash256("bc544c2baba832600013bd5d1983f592e9557d04b0fb5ef7a100434a5fc8d52a");
    const ethash::hash256 mix_hash =
        to_hash256("94cd4e844619ee20989578276a0a9046877d569d37ba076bf2e8e34f76189dea");
    const uint64_t nonce = 0x4617a20003ba3f25;
    const uint64_t target = 0x0000000000001a5b + 1;

    const int epoch_number = ethash::get_epoch_number(block_number);

    // This should create the light cache.
    ethash::managed::get_epoch_context(epoch_number);

    for (auto _ : state)
    {
        auto& context = ethash::managed::get_epoch_context(epoch_number);
        ethash::verify(context, header_hash, mix_hash, nonce, target);
    }
}
BENCHMARK(verify_managed)->Threads(1)->Threads(2)->Threads(4)->Threads(8);


BENCHMARK_MAIN();
