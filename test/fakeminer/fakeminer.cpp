// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash.hpp>

#include <chrono>
#include <future>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


using namespace std::chrono;
using timer = std::chrono::steady_clock;

int main(int argc, const char* argv[])
{
    size_t num_iterations = 10000;
    int epoch = 0;
    size_t num_threads = 1;
    uint64_t start_nonce = 0;
    bool light = false;

    for (int i = 0; i < argc; ++i)
    {
        const std::string arg{argv[i]};

        if (arg == "--light")
            light = true;
        else if (arg == "-i" && i + 1 < argc)
            num_iterations = std::stoul(argv[++i]);
        else if (arg == "-e" && i + 1 < argc)
            epoch = std::stoi(argv[++i]);
        else if (arg == "-t" && i + 1 < argc)
            num_threads = std::stoul(argv[++i]);
        else if (arg == "-n" && i + 1 < argc)
            start_nonce = std::stoul(argv[++i]);
    }

    // clang-format off
    std::cout << "fakeminer benchmark"
              << "\n  iterations:  " << num_iterations
              << "\n  threads:     " << num_threads
              << "\n  start nonce: " << start_nonce
              << std::endl;
    // clang-format on


    const ethash::hash256 header_hash{};
    const size_t iterations_per_thread = num_iterations / num_threads;

    auto* context = ethash::create_epoch_context(epoch);
    if (!light)
        ethash::init_full_dataset(*context);

    using runner_fn = std::function<void(const ethash::hash256&, uint64_t, size_t)>;
    const runner_fn full_runner = [context](const ethash::hash256& header_hash,
                                      uint64_t start_nonce, size_t iterations) {
        ethash::search(*context, header_hash, 0, start_nonce, iterations);
    };
    const runner_fn light_runner = [context](const ethash::hash256& header_hash,
                                       uint64_t start_nonce, size_t iterations) {
        ethash::search_light(*context, header_hash, 0, start_nonce, iterations);
    };

    const auto& runner = light ? light_runner : full_runner;


    std::vector<std::future<void>> futures;
    auto start_time = timer::now();

    for (size_t t = 0; t < num_threads; ++t)
    {
        futures.emplace_back(std::async(
            std::launch::async, runner, header_hash, start_nonce, iterations_per_thread));
        start_nonce += iterations_per_thread;
    }

    for (auto& future : futures)
        future.wait();

    ethash::destroy_epoch_context(context);

    auto ms = duration_cast<milliseconds>(timer::now() - start_time).count();
    auto hps = static_cast<decltype(ms)>(num_iterations) * 1000 / ms;

    std::cout << ((ms + 999) / 1000) << " s\n" << hps << " H/s\n";
    return 0;
}
