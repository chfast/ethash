// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash.hpp>
#include <chrono>
#include <iostream>

using namespace std::chrono;
using timer = std::chrono::steady_clock;

int main()
{
    constexpr size_t n = 5000;

    const ethash::hash256 header_hash{};

    auto context = ethash::epoch_context{0};

    auto start_time = timer::now();
    ethash::search(context, header_hash, 0, 0, n);
    auto ms = duration_cast<milliseconds>(timer::now() - start_time).count();
    auto hps = n * 1000 / ms;

    std::cout << ((ms + 999) / 1000) << " s\n" << hps << " H/s\n";
    return 0;
}
