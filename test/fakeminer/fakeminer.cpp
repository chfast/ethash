// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


using namespace std::chrono;
using timer = std::chrono::steady_clock;

namespace
{
class ethash_interface
{
public:
    virtual ~ethash_interface() noexcept = default;

    virtual void search(
        const ethash::hash256& header_hash, uint64_t nonce, size_t iterations) const noexcept = 0;
};

class ethash_light : public ethash_interface
{
    ethash::epoch_context_ptr context;

public:
    explicit ethash_light(int epoch_number) : context{ethash::create_epoch_context(epoch_number)} {}

    void search(const ethash::hash256& header_hash, uint64_t nonce, size_t iterations) const
        noexcept override
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

    void search(const ethash::hash256& header_hash, uint64_t nonce, size_t iterations) const
        noexcept override
    {
        ethash::search(*context, header_hash, 0, nonce, iterations);
    }
};


std::atomic<int> num_hashes{0};

void worker(const ethash_interface& ei, const ethash::hash256& header_hash, uint64_t start_nonce,
    int iterations, int work_size)
{
    for (int i = 0; i < iterations; i += work_size)
    {
        // FIXME: Change iterations arg in search().
        ei.search(
            header_hash, start_nonce + static_cast<uint64_t>(i), static_cast<size_t>(work_size));

        num_hashes.fetch_add(work_size, std::memory_order_relaxed);
    }
}
}  // namespace

int main(int argc, const char* argv[])
{
    int num_iterations = 10000;
    int work_size = 100;
    int epoch = 0;
    int num_threads = 1;
    uint64_t start_nonce = 0;
    bool light = false;

    for (int i = 0; i < argc; ++i)
    {
        const std::string arg{argv[i]};

        if (arg == "--light")
            light = true;
        else if (arg == "-i" && i + 1 < argc)
            num_iterations = std::stoi(argv[++i]);
        else if (arg == "-e" && i + 1 < argc)
            epoch = std::stoi(argv[++i]);
        else if (arg == "-t" && i + 1 < argc)
            num_threads = std::stoi(argv[++i]);
        else if (arg == "-n" && i + 1 < argc)
            start_nonce = std::stoul(argv[++i]);
    }

    // clang-format off
    std::cout << "fakeminer benchmark"
              << "\n  iterations:  " << num_iterations
              << "\n  threads:     " << num_threads
              << "\n  start nonce: " << start_nonce
              << "\n\n";
    // clang-format on


    const ethash::hash256 header_hash{};
    const int iterations_per_thread = num_iterations / num_threads;

    std::unique_ptr<ethash_interface> ei{
        light ? static_cast<ethash_interface*>(new ethash_light{epoch}) : new ethash_full{epoch}};

    std::vector<std::future<void>> futures;

    for (int t = 0; t < num_threads; ++t)
    {
        futures.emplace_back(std::async(std::launch::async, worker, std::ref(*ei), header_hash,
            start_nonce, iterations_per_thread, work_size));
        start_nonce += static_cast<uint64_t>(iterations_per_thread);
    }

    std::cout << "Hashrate:\n      current      average\n";

    int all_hashes = 0;
    auto start_time = timer::now();
    auto time = start_time;
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        int current_hashes = num_hashes.exchange(0, std::memory_order_relaxed);
        all_hashes += current_hashes;

        auto now = timer::now();
        auto current_duration = duration_cast<milliseconds>(now - time).count();
        auto all_duration = duration_cast<milliseconds>(now - start_time).count();
        time = now;

        auto current_khps = double(current_hashes) / double(current_duration);
        auto average_khps = double(all_hashes) / double(all_duration);

        std::cout << std::fixed << "  " << std::setw(6) << std::setprecision(2) << current_khps
                  << " kh/s  " << std::setw(6) << std::setprecision(2) << average_khps << " kh/s\n";

        if (all_hashes >= num_iterations)
            break;
    }

    for (auto& future : futures)
        future.wait();

    auto ms = duration_cast<milliseconds>(timer::now() - start_time).count();
    auto hps = static_cast<decltype(ms)>(num_iterations) * 1000 / ms;

    std::cout << ((ms + 999) / 1000) << " s\n" << hps << " H/s\n";
    return 0;
}
