// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "utils.hpp"

namespace ethash
{
bool is_prime(size_t number) noexcept
{
    if (number <= 1)
        return false;

    if (number % 2 == 0 && number > 2)
        return false;

    for (size_t d = 3; (d * d) < number; d += 2)
    {
        if (number % d == 0)
            return false;
    }

    return true;
}
}