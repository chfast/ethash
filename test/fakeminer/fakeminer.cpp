// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash.hpp>

#include <chrono>
#include <future>
#include <iostream>
#include <unordered_map>


using namespace std::chrono;
using timer = std::chrono::steady_clock;

int main(int argc, const char* argv[])
{
    size_t num_iterations = 5000;
    uint32_t epoch = 0;
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
            epoch = static_cast<uint32_t>(std::stoul(argv[++i]));
        else if (arg == "-t" && i + 1 < argc)
            num_threads = std::stoul(argv[++i]);
        else if (arg == "-n" && i + 1 < argc)
            start_nonce = std::stoul(argv[++i]);
    }

    const ethash::hash256 header_hash{};
    const size_t iterations_per_thread = num_iterations / num_threads;

    auto context = ethash::epoch_context{epoch};

    auto start_time = timer::now();

    if (light)
        ethash::search_light(context, header_hash, 0, start_nonce, num_iterations);
    else
    {
        ethash::init_full_dataset(context);

        std::vector<std::future<void>> futures;

        for (size_t t = 0; t < num_threads; ++t)
        {
            futures.emplace_back(std::async(
                std::launch::async, [&context, header_hash, start_nonce, iterations_per_thread] {
                    ethash::search(context, header_hash, 0, start_nonce, iterations_per_thread);
                }));
            start_nonce += iterations_per_thread;
        }

        for (auto& future : futures)
            future.wait();
    }

    auto ms = duration_cast<milliseconds>(timer::now() - start_time).count();
    auto hps = num_iterations * 1000 / ms;

    std::cout << ((ms + 999) / 1000) << " s\n" << hps << " H/s\n";
    return 0;
}
