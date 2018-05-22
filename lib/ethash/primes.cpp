// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "primes.hpp"

#include <cstdint>

namespace ethash
{
bool is_prime(int number) noexcept
{
    if (number <= 1)
        return false;

    if (number % 2 == 0 && number > 2)
        return false;

    // Check factors up to sqrt(number).
    // To avoid computing sqrt, compare d*d <= number with 64-bit precision.
    for (int d = 3; int64_t(d) * int64_t(d) <= int64_t(number); d += 2)
    {
        if (number % d == 0)
            return false;
    }

    return true;
}

int find_largest_prime(int upper_bound) noexcept
{
    int n = upper_bound;

    if (n < 2)
        return 0;

    if (n == 2)
        return 2;

    /* If even number, skip it. */
    if (n % 2 == 0)
        --n;

    /* Test descending odd numbers. */
    while (!is_prime(n))
        n -= 2;

    return n;
}
}  // namespace ethash
