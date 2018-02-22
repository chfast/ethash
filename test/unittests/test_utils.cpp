// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/utils.hpp>

#include <gtest/gtest.h>


class is_prime_positive_test : public ::testing::TestWithParam<size_t>
{
};

class is_prime_negative_test : public ::testing::TestWithParam<size_t>
{
};


TEST_P(is_prime_positive_test, primes)
{
    const size_t number = GetParam();
    EXPECT_TRUE(ethash::is_prime(number));
}
INSTANTIATE_TEST_CASE_P(primes, is_prime_positive_test,
    testing::Values(2, 3, 17, 19, 577, 2069, 3011, 7919, 142540787, 2147483647));

TEST_P(is_prime_negative_test, nonprimes)
{
    const size_t number = GetParam();
    EXPECT_FALSE(ethash::is_prime(number));
}
INSTANTIATE_TEST_CASE_P(nonprimes, is_prime_negative_test,
    testing::Values(0, 1, 4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 32, 512, 2048, 8192));
