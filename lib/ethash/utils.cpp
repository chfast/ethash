// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "utils.hpp"

#include <cassert>

namespace ethash
{
bool is_prime(size_t number) noexcept
{
    // FIXME: For 64-bit numbers this implementation is not feasible, but we
    //        need more than 32-bits anyway (size_t is bad type).

    if (number <= 1)
        return false;

    if (number % 2 == 0 && number > 2)
        return false;

    for (size_t d = 3; (d * d) <= number; d += 2)
    {
        if (number % d == 0)
            return false;
    }

    return true;
}

size_t find_largest_prime(size_t upper_bound) noexcept
{
    assert(upper_bound > 2);

    size_t n = upper_bound;

    // If even number, skip it.
    if (n % 2 == 0)
        --n;

    // Test descending odd numbers.
    while (!is_prime(n))
        n -= 2;

    return n;
}
}
