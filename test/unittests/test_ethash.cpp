// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash.hpp>
#include <ethash/ethash-internal.hpp>

#include "helpers.hpp"

#include <gtest/gtest.h>

using namespace ethash;

class calculate_light_cache_size_test
  : public ::testing::TestWithParam<std::pair<uint32_t, uint64_t>>
{
};

class calculate_full_dataset_size_test
  : public ::testing::TestWithParam<std::pair<uint32_t, uint64_t>>
{
};

TEST_P(calculate_light_cache_size_test, test)
{
    uint32_t epoch_number;
    uint64_t expected_size;
    std::tie(epoch_number, expected_size) = GetParam();

    uint64_t size = ethash::calculate_light_cache_size(epoch_number);
    EXPECT_EQ(size, expected_size);
}

/// Test pairs for calculate_light_cache_size() are taken from generated ethash
/// light cache sizes, picked at random.
/// See https://github.com/ethereum/wiki/wiki/Ethash#data-sizes.
static std::pair<uint32_t, uint64_t> light_cache_sizes[] = {{0, 16776896}, {14, 18611392},
    {17, 19004224}, {56, 24116672}, {158, 37486528}, {203, 43382848}, {211, 44433344},
    {272, 52427968}, {350, 62651584}, {412, 70778816}, {464, 77593664}, {530, 86244416},
    {656, 102760384}, {657, 102890432}, {658, 103021888}, {739, 113639104}, {751, 115212224},
    {798, 121372352}, {810, 122945344}, {862, 129760832}, {882, 132382528}, {920, 137363392},
    {977, 144832448}, {1093, 160038464}, {1096, 160430656}, {1119, 163446592}, {1125, 164233024},
    {1165, 169475648}, {1168, 169866304}, {1174, 170655424}, {1211, 175504832}, {1244, 179830208},
    {1410, 201588544}, {1418, 202636352}, {1436, 204996544}, {1502, 213645632}, {1512, 214956992},
    {1535, 217972672}, {1538, 218364736}, {1554, 220461632}, {1571, 222690368}, {1602, 226754368},
    {1621, 229243328}, {1630, 230424512}, {1698, 239335232}, {1746, 245628736}, {1790, 251395136},
    {1818, 255065792}, {1912, 267386432}, {1928, 269482688}, {1956, 273153856}, {2047, 285081536}};
INSTANTIATE_TEST_CASE_P(
    light_cache_sizes, calculate_light_cache_size_test, testing::ValuesIn(light_cache_sizes));


TEST_P(calculate_full_dataset_size_test, test)
{
    uint32_t epoch_number;
    uint64_t expected_size;
    std::tie(epoch_number, expected_size) = GetParam();

    uint64_t size = ethash::calculate_full_dataset_size(epoch_number);
    EXPECT_EQ(size, expected_size);
}

/// Test pairs for calculate_full_dataset_size() are taken from generated ethash
/// full dataset sizes, picked at random.
/// See https://github.com/ethereum/wiki/wiki/Ethash#data-sizes.
static std::pair<uint32_t, uint64_t> full_dataset_sizes[] = {{0, 1073739904}, {14, 1191180416},
    {17, 1216345216}, {56, 1543503488}, {158, 2399139968}, {203, 2776625536}, {211, 2843734144},
    {272, 3355440512}, {350, 4009751168}, {412, 4529846144}, {464, 4966054784}, {530, 5519703424},
    {656, 6576662912}, {657, 6585055616}, {658, 6593443456}, {739, 7272921472}, {751, 7373585792},
    {798, 7767849088}, {810, 7868512384}, {862, 8304715136}, {882, 8472492928}, {920, 8791260032},
    {977, 9269411456}, {1093, 10242489472}, {1096, 10267656064}, {1119, 10460589952},
    {1125, 10510923392}, {1165, 10846469248}, {1168, 10871631488}, {1174, 10921964672},
    {1211, 11232345728}, {1244, 11509169024}, {1410, 12901672832}, {1418, 12968786816},
    {1436, 13119782528}, {1502, 13673430656}, {1512, 13757316224}, {1535, 13950253696},
    {1538, 13975417216}, {1554, 14109635968}, {1571, 14252243072}, {1602, 14512291712},
    {1621, 14671675264}, {1630, 14747172736}, {1698, 15317596544}, {1746, 15720251264},
    {1790, 16089346688}, {1818, 16324230016}, {1912, 17112759424}, {1928, 17246976896},
    {1956, 17481857408}, {2047, 18245220736}};
INSTANTIATE_TEST_CASE_P(
    full_dataset_sizes, calculate_full_dataset_size_test, testing::ValuesIn(full_dataset_sizes));


TEST(calculate_seed_test, zero)
{
    hash256 s0 = calculate_seed(0);
    EXPECT_EQ(s0.words[0], 0);
    EXPECT_EQ(s0.words[1], 0);
    EXPECT_EQ(s0.words[2], 0);
    EXPECT_EQ(s0.words[3], 0);
}

TEST(calculate_seed_test, one)
{
    hash256 seed = calculate_seed(1);
    std::string sh = to_hex(seed);
    EXPECT_EQ(sh, "290decd9548b62a8d60345a988386fc84ba6bc95484008f6362f93160ef3e563");
}

TEST(calculate_seed_test, current)
{
    hash256 seed = calculate_seed(171);
    std::string sh = to_hex(seed);
    EXPECT_EQ(sh, "a9b0e0c9aca72c07ba06b5bbdae8b8f69e61878301508473379bb4f71807d707");
}

TEST(calculate_seed_test, last)
{
    hash256 seed = calculate_seed(2048);
    std::string sh = to_hex(seed);
    // FIXME: Compare with legacy ethash.
    EXPECT_EQ(sh, "20a7678ca7b50829183baac2e1e3c43fa3c4bcbc171b11cf5a9f30bebd172920");
}