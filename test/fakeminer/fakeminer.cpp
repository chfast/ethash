// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <chrono>
#include <ethash/ethash.hpp>
#include <iostream>
#include <unordered_map>


using namespace std::chrono;
using timer = std::chrono::steady_clock;

int main(int argc, const char* argv[])
{
    size_t n = 5000;
    uint32_t epoch = 0;
    bool light = false;

    for (int i = 0; i < argc; ++i)
    {
        const std::string arg{argv[i]};

        if (arg == "--light")
            light = true;
        else if (arg == "-n" && i + 1 < argc)
            n = std::stoul(argv[++i]);
        else if (arg == "-e" && i + 1 < argc)
            epoch = static_cast<uint32_t>(std::stoul(argv[++i]));
    }

    const ethash::hash256 header_hash{};

    auto context = ethash::epoch_context{epoch};

    auto start_time = timer::now();

    if (light)
        ethash::search_light(context, header_hash, 0, 0, n);
    else
    {
        ethash::init_full_dataset(context);
        ethash::search(context, header_hash, 0, 0, n);
    }

    auto ms = duration_cast<milliseconds>(timer::now() - start_time).count();
    auto hps = n * 1000 / ms;

    std::cout << ((ms + 999) / 1000) << " s\n" << hps << " H/s\n";
    return 0;
}
