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

class ethash_interface
{
public:
    virtual ~ethash_interface() noexcept = default;

    virtual void search(
        const ethash::hash256& header_hash, uint64_t nonce, size_t iterations) noexcept = 0;
};

class ethash_light : public ethash_interface
{
    ethash::epoch_context_ptr context;

public:
    explicit ethash_light(int epoch_number) : context{ethash::create_epoch_context(epoch_number)} {}

    void search(
        const ethash::hash256& header_hash, uint64_t nonce, size_t iterations) noexcept override
    {
        ethash::search_light(*context, header_hash, 0, nonce, iterations);
    }
};

class ethash_full : public ethash_interface
{
    ethash::epoch_context_full_ptr context;

public:
    explicit ethash_full(int epoch_number)
      : context{ethash::create_epoch_context_full(epoch_number)}
    {}

    void search(
        const ethash::hash256& header_hash, uint64_t nonce, size_t iterations) noexcept override
    {
        ethash::search(*context, header_hash, 0, nonce, iterations);
    }
};

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

    std::unique_ptr<ethash_interface> ei{
        light ? static_cast<ethash_interface*>(new ethash_light{epoch}) : new ethash_full{epoch}};

    std::vector<std::future<void>> futures;
    auto start_time = timer::now();

    for (size_t t = 0; t < num_threads; ++t)
    {
        futures.emplace_back(std::async(std::launch::async,
            [=, &ei] { ei->search(header_hash, start_nonce, iterations_per_thread); }));
        start_nonce += iterations_per_thread;
    }

    for (auto& future : futures)
        future.wait();

    auto ms = duration_cast<milliseconds>(timer::now() - start_time).count();
    auto hps = static_cast<decltype(ms)>(num_iterations) * 1000 / ms;

    std::cout << ((ms + 999) / 1000) << " s\n" << hps << " H/s\n";
    return 0;
}
