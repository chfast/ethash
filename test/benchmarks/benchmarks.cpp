// Copyright 2018 Pawel Bylica.
// Governed by the Apache License, Version 2.0. See the LICENSE file.

#include "../unittests/helpers.hpp"

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
    const int epoch_number = state.range(0);

    for (auto _ : state)
    {
        auto seed = ethash::calculate_seed(epoch_number);
        benchmark::DoNotOptimize(seed.bytes);
    }
}
BENCHMARK(seed)->Arg(1)->Arg(10)->Arg(100)->Arg(1000);


static void light_cache(benchmark::State& state)
{
    const int epoch_number = state.range(0);
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
    static auto deleter = [](ethash::epoch_context* context) {
        ethash::destroy_epoch_context(context);
    };
    static std::unique_ptr<ethash::epoch_context, decltype(deleter)> ctx{
        ethash::create_epoch_context(0), deleter};

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

    static auto deleter = [](ethash::epoch_context* context) {
        ethash::destroy_epoch_context(context);
    };
    static std::unique_ptr<ethash::epoch_context, decltype(deleter)> ctx{
        ethash::create_epoch_context(ethash::get_epoch_number(block_number)), deleter};

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

    // This should create the light cache.
    bool v = ethash::managed::verify(block_number, header_hash, mix_hash, nonce, target);

    if (!v)
        state.SkipWithError("Invalid input data for validation");

    for (auto _ : state)
        ethash::managed::verify(block_number, header_hash, mix_hash, nonce, target);
}
BENCHMARK(verify_managed)->Threads(1)->Threads(2)->Threads(4)->Threads(8);


BENCHMARK_MAIN()
