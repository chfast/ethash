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

TEST(ethash, version)
{
    std::string expected_version{"0.1.0.dev0"};
    EXPECT_EQ(version(), expected_version);
}

TEST(ethash, light_cache)
{
    struct light_cache_test_case
    {
        struct item_test
        {
            size_t index;
            const char* hash_hex;
        };

        uint32_t epoch_number;
        std::vector<item_test> item_tests;
    };

    // clang-format off
    light_cache_test_case test_cases[] = {
        {0,
            {
                {0,      "5e493e76a1318e50815c6ce77950425532964ebbb8dcf94718991fa9a82eaf37658de68ca6fe078884e803da3a26a4aa56420a6867ebcd9ab0f29b08d1c48fed"},
                {1,      "47bcbf5825d4ba95ed8d9074291fa2242cd6aabab201fd923565086c6d19b93c6936bba8def3c8e40f891288f9926eb87318c30b22611653439801d2a596a78c"},
                {52459,  "a1cfe59d6fcb93fe54a090f4d440472253b1d18962acc64a949c955ee5785c0a577bbbe8e02acdcc2d829df28357ec6ba7bb37d5026a6bd2c43e2f990b416fa9"},
                {176162, "b67128c2b592f834aaa17dad7990596f62cb85b11b3ee7072fc9f8d054366d30bed498a1d2e296920ce01ec0e32559c434a6a20329e23020ecce615ff7938c23"},
                {194590, "05ed082c5cb2c511aceb8ecd2dc785b46f77122e788e3dacbc76ecee5050b83be076758ca9aa6528e384edbf7c3ccb96ca0622fcbdd643380bd9d4dd2e293a59"},
                {262137, "4615b9c6e721f9c7b36ab317e8f125cc4edc42fabf391535da5ef15090182c2523a007d04aeca6fd1d3f926f5ae27a65400404fcdc80dd7da1d46fdaac030d06"},
                {262138, "724f2f86c24c487809dc3897acbbd32d5d791e4536aa1520e65e93891a40dde5887899ffc556cbd174f426e32ae2ab711be859601c024d1514b29a27370b662e"},
            }},
        {171,
            {
                {0,      "bb7f72d3813a9a9ad14e6396a3be9edba40197f8c6b60a26ca188260227f5d287616e9c93da7de35fd237c191c36cdcc00abd98dfcacd11d1f2544aa52000917"},
                {1,      "460036cc618bec3abd406c0cb53182c4b4dbbcc0266995d0f7dd3511b5a4d599405cb6cc3aff5874a7cfb87e71688ba0bb63c4aa756fd36676aa947d53c9a1b1"},
                {173473, "d5cd1e7ced40ad920d977ff3a28e11d8b42b556c44e3cf48e4cce0a0854a188182f971db468756f951ed9b76c9ecf3dbbdd0209e89febe893a88785e3c714e37"},
                {202503, "609227c6156334b0633526c032ab68d82414aee92461e60e75c5751e09e5c4d645e13e1332d2cddd993da5b5b872b18b5c26eabef39acf4fc610120685aa4b24"},
                {227478, "fb5bd4942d7da4c80dc4075e5dc121a1fb1d42a7feae844d5fe321cda1db3dbdd4035dadc5b69585f77580a61260dc36a75549995f6bcba5539da8fa726c1dd5"},
                {612347, "7a3373b5c50b8950de5172b0f2b7565ecaf00e3543de972e21122fb31505085b196c6be11738d6fce828dac744159bbd62381beddfcbd00586b8a84c6c4468c8"},
                {612348, "ec45073bd7820fe58ea29fa89375050cfb1da7bdb17b79f20f8e427bef1cdc0976d1291597fece7f538e5281a9d8df3f0b842bb691ade89d3864dfa965c7e187"},
            }},
        {2047,
            {
                {0,       "e887f74775c5ac3f2ed928d74dde3d8b821e9b9f251f6bb66ccc817c3c52e01b319d5cf394b0af867c871acca3375c0118ffbdafe4d7b7d23d7361cf14ed1458"},
                {1,       "9e6132d61fab636a6b95207c9d4437beabaa683d50a03a49e0fc79c1d375a067f2ced6d3be3f26e7e864a156035ad80d1a5295bf5e2be71722ed8f2708c50997"},
                {574988,  "d5f763c03e8343d4e9e55fffb45045bb25d3f9d50175453bec652e469f29419058e33bcddf8a39ba2cc84a8bf52ed58d48bcbe052b41827ed3c14b281bf088af"},
                {2778440, "d1eafd2b51d3c4485d1959ed49434c5e0684bd14e58e8b77244fca33939f64f396d205df1fb19b342f9b8605e4098ae6244d06a0a564cdd150f8e3a06681b5fb"},
                {3292172, "004aa90c2ffae1ac6c5573222a214c05a22c5d4d4db35747a945d7903aa1867ae57783cafd350fdaf23441da020403810213c3724be37e9353909d9a5283819d"},
                {4454397, "5a74ae8affebf2df925265b8467ae3f71b61814636d34bee677eeb308f28c7c18a0e6394c2c20b3026ac33386c0a2af38f92a1cc1633ea66ceefbf1e9090da57"},
                {4454398, "0504239bf95d918c1f09b9f05659376a4dd461f3b5025ab05fefd1fb6c41f08da90e38dcff6861bfe28ab3e09997b9f373f017c13f30818670c92267d5b91d55"},
            }},
    };
    // clang-format on


    for (const auto& t : test_cases)
    {
        const uint64_t size = calculate_light_cache_size(t.epoch_number);
        const auto seed = calculate_seed(t.epoch_number);
        const auto light_cache = make_light_cache(size, seed);

        for (auto& u : t.item_tests)
        {
            ASSERT_LT(u.index, light_cache.size());
            EXPECT_EQ(to_hex(light_cache[u.index]), u.hash_hex)
                << "epoch: " << t.epoch_number << " item: " << u.index;
        }
    }
}
