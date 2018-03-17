// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/ethash.hpp>
#include <ethash/ethash-internal.hpp>
#include <ethash/params.hpp>

#include "helpers.hpp"

#include <gtest/gtest.h>
#include <future>

using namespace ethash;

namespace
{
    /// Creates the epoch context of the correct size but filled with fake data.
    epoch_context create_epoch_context_mock(uint32_t epoch_number)
    {
        hash512 fill;
        std::fill_n(fill.words, 8, 0xe14a54aaaaaaaaaa);
        const size_t cache_size = calculate_light_cache_size(epoch_number);

        epoch_context context = {};
        context.cache = light_cache(cache_size, fill);
        context.full_dataset_size = calculate_full_dataset_size(epoch_number);
        return context;
    }
}

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
        auto* context = create_epoch_context(t.epoch_number);

        for (auto& u : t.item_tests)
        {
            ASSERT_LT(u.index, context->cache.size());
            EXPECT_EQ(to_hex(context->cache[u.index]), u.hash_hex)
                << "epoch: " << t.epoch_number << " item: " << u.index;
        }

        destroy_epoch_context(context);
    }
}

TEST(ethash, dataset_partial_items)
{
    struct full_dataset_item_test_case
    {
        size_t index;
        const char* hash_hex;
    };

    // Number of items in the fake light cache.
    static constexpr size_t fake_cache_items = 6133493;

    // clang-format off
    full_dataset_item_test_case test_cases[] = {
        {0,          "4764f1f61f71532de83e1231658fce600c5bec86ab02815caaf08f51d7217262af084f8708dcec54388404d47d88626b7d3e565efa043ac5a2bd18aeab2dd118"},
        {1,          "a629c1513c01cd6eba6db3c3f836ea94c5905615c8d3d2a608105b38affe78b39059efca6ac97e98eb2a05106fe488ea0ba7eb18e1b1ebc5c43d11241010d8e0"},
        {13,         "efce2a208a0beb57fb1789a9158da9a4eafbb5a50aa17c8ef352cf339ae1db0ddeed32e41d7a74a7a870d255714d93f906c5a6a8ff3129931d47e04ce2da5a32"},
        {171,        "973cdf0cbb58c21352d9aaf384141af3946f17016ad5b02bc1ab581e262534dc1bad6bebf91cf874733b798b3d2098dafeeb55629a6bc93cc1ec25fa6ca78927"},
        {571,        "9d9688735fb83b4f1efec9a2863bcd494f97c337b72cbbb35a86484ff3013417751547cdbc641851a02240745f671aae943654d5b54aafb258cb440fea107aa5"},
        {620,        "cf121f8cdc19828904c0ff761ee1515cc231f250dfff206b25067f597418921f1b63455376121171fb657ad21a8135d8da26ee7baf5bfc302a77e89b57e3a56a"},
        {514079,     "38f5350e9b28531f80355e47260c09bfc3f6d6da07050343fed9c08902e2347065584dc30a08228b0d2f780e4c157fffbf0b2177c546192f824ca3531632a4b7"},
        {852881,     "992879fea4525e8c44b1c2eb6a885b32a24efd4eacf6353604d4c688ba56d15c5b50c04672d583f35ac24a8509b8f925a96f5078d41e3675149742da2e992dee"},
        {6133492,    "713275b1fa7dc300093efa48b8a6047776832ee747cb80784669157d8b35d52665f28eb57f3148e9b6e46b60e47eb587d4495febf4734e6ae8387aa3e80db94e"},
        {6133493,    "009b0b6154046fc82c78d3e712924227415b0663e9c5fbb5310e3580e36861cc381dd135b24b8e746018fc9a4dcc2962fb432bbe2f5abfabc09b182f15df6b50"},
        {211322105,  "95aee5086722951be65ae6440aaebd0dda6d17cdb3057cdb99f917cfae36c6fba7be47084d90224239cce7c763196eec27dfda31bb786e284b07e540ae3f4162"},
        {740620450,  "cc65a8f1ad9d6ca3c8df787a2ff0191e05b1307fc2dbe5aff7ec2d2549ba0d4f5416f82fccb005245fef876643c840f66d9df7d596826bd22a2dc7c4952901c2"},
        {4294967295, "b47377a0c48b74caad00ea4c7e486cab7e0ffc5b7e08b67f61e93b68f3907b8a8e09b3ff57c1f7a0dc886ebb32a868926e0a066c44a53be707affaeddf7f039d"},
    };
    // clang-format on


    hash512 fake_item;
    std::fill(std::begin(fake_item.bytes), std::end(fake_item.bytes), 0b1010101);
    light_cache fake_cache(fake_cache_items, fake_item);

    for (const auto& t : test_cases)
    {
        const hash512 item = calculate_dataset_item_partial(fake_cache, t.index);
        EXPECT_EQ(to_hex(item), t.hash_hex) << "index: " << t.index;
    }
}

TEST(ethash, dataset_items)
{
    struct full_dataset_item_test_case
    {
        size_t index;
        const char* hash1_hex;
        const char* hash2_hex;
    };

    // Number of items in the fake light cache.
    static constexpr size_t fake_cache_items = 6133493;

    // clang-format off
    full_dataset_item_test_case test_cases[] = {
        {0,
            "4764f1f61f71532de83e1231658fce600c5bec86ab02815caaf08f51d7217262af084f8708dcec54388404d47d88626b7d3e565efa043ac5a2bd18aeab2dd118",
            "a629c1513c01cd6eba6db3c3f836ea94c5905615c8d3d2a608105b38affe78b39059efca6ac97e98eb2a05106fe488ea0ba7eb18e1b1ebc5c43d11241010d8e0"},
        {1,
            "4b52a62634fe25a9c0b21bde997f6a767f2bfd022e516edb2349a5e044059b1ad549e0db9c25914cdcd14e6ef37a68ee4fe3f4411f42fa604b000a6723556bc4",
            "0c9eddd366c87cdd6f9cdec373f227b2b922fd8024019b4c35c9337cb9e7d8d5b0fbd1808e026a4b7098fd23e73036f1ffe22eb189bc10c489318ccaf14ee43f"},
        {13,
            "cc1e4ad4e7e905109f4dc2b21857d54802c9e550f8724601b0ef5ac7255aa9d43cf39c8a5e64e55e55a9a2fe6dfbfd4e6c621cadf731d6bb98df1a370551d498",
            "3329b0d19d3021f706a703d9b6c983523cfba653514f9806e2bed3996d7e290602a33b5372bec603ef3cd3febfe45fbdd420b89315ad87b14b6237799c8535e0"},
        {171,
            "0119b5d5886550162061180ba620044b93bf280081c0b9275343271c010a317a1ebaf492f2dc4b9d2dd62043d8cdf2dc621fc42f2eda9171a2c3f4f260cf1d70",
            "f30adcb55ff0aff723d6b27b6467ee56ffafced8c6ed703e3532fb50bf61e752448c9c24e727b1352ab429f9aecdeca0fc019b152a67f5cec530d2a47b7438e3"},
        {571,
            "26be82d9c86e697afa5708bb457439d2874c11ee3b700e614f141b5689f2408981d13193e367f33279f667cfa8403c8d16b8c767b69663fe1a8ed3c756b164c7",
            "790e7cc49c225a5a34cf307a41bb7e07a9145665b3e5161d65ef291dfec711eac7af401aac1908c85f476743dd796a886a1d822883421725832d9b4b3a0e3ae7"},
        {620,
            "37fc516fb90d1c924a73475fb4fb806ac2d5562c7506c54542570ef9ba4eafa76ae97f299e6f48cdc8d6b7b54e01474c927fd3ceec73136696b8b0463f4a962a",
            "0e1581c55baba97493edf71f7e1363710d89eecb77e058f299d7aae5fe97a05a3f0d1b01c9e289bd6a5084a5fe9a22502299e8ceb9d7463465879b2834aa01f9"},
        {514079,
            "5189058ba850e4278a120f9f14b0220a107d8f714680cd5fcc6a99235264bc3f83cd90e10d48a59567c44cf40679aa6fec8af70cba0ede5bff26e71f1f5c2a9b",
            "acef6860072fe7d2969862a96f0036a790a28a617e6953f69fefa04941f16f616005d3e0148e4c95e66d881bd0f439896ae97a5d0108df30b6cf3f3a1bfbac16"},
        {852881,
            "95f9eeafb89861f84fd494500d9a4834c07c018bf23735af846c6de710dc661f75d65fb58965a895dc3696547b4f66143b67e7ed46e735ce0179e76b5795d1d1",
            "3fa43ef4cee22359664d872169338f4a037eb62eddef12e2f08552fd518ce16c6f014df131435878577abdf74f590c48ab6b0bc8fffc513ac70900accb7cf53a"},
        {6133492,
            "9d3828b4a054d2979f180380fa6cb254c56926b9a3cb5a5e684595f97024e474af2a5150bd2dd1da439e9ed5bbd5c30c49b31f758f4c973eb3efdf7dc573c0d8",
            "6b7dd1c31cf0d79e6cf5879ff1b67205c8a66e0320000d2f5ae93b5e23f46bd389892ba4db620fa33d9d97233b7dd078471960500d214dfdfb786dc1a6cb5944"},
        {6133493,
            "7ca74fcd1ac8627d5e1b2c2c5313b465761ae8f04a7bb141522f3dfc37dcd81880c185fc1b5c270e1ab8356189a43deca47f51a31956b22aa2a47c4c85e6b270",
            "d18db7d0119e91fb50866507541e310a58c243df6705eaa38a0d345023019e7cbb1014e889d28ec72a3332b974227d7299df48da2a720e4a92b1856c90b6dd2c"},
        {211322105,
            "415f7ec751e8a790dc5f1c0937026665014366ecb6d04e1b54f33482978d29d84a32d977a774138fd76333378485dd75811f725ef0e5de79aba5b3ad38060c99",
            "1e98c227c3ff62d9bd4a7cfdfd5af8c461f1d840ff5d60bcd4fd0eae8927f956100dbab2e4e947a785a1aaa0132fbb05a95402c57ef7fd9184ebcd82c39e1ae0"},
        {740620450,
            "dba6544efccd33440ae256cf0b3aa4182e863d8cab841c8c3195dd67464ba8acffa6d28426e8dc519aa49f1898e82ceabb0a3b6cd30be6a51b57ca3d673dbcaa",
            "b3201249c850f81327920b653f7c6e098ab332caf81ff5c74072dd87a1c2688944e742c36f397899be219ccddea6125c33f5c42b250bdbaf5e4d6e5638334277"},
        {4294967295,
            "63c457a719afc75260325f9de19e2979f9fa330656d9651a2e80c6a809d789dd71d2dbed91258f8bd4d872b8ca73bd7b7ddd39644f1b25e8eba628741d454daf",
            "b47377a0c48b74caad00ea4c7e486cab7e0ffc5b7e08b67f61e93b68f3907b8a8e09b3ff57c1f7a0dc886ebb32a868926e0a066c44a53be707affaeddf7f039d"},
    };
    // clang-format on


    hash512 fake_item;
    std::fill(std::begin(fake_item.bytes), std::end(fake_item.bytes), 0b1010101);
    light_cache fake_cache(fake_cache_items, fake_item);

    // Mock the epoch context.
    epoch_context context = create_epoch_context_mock(0);
    context.cache = std::move(fake_cache);

    for (const auto& t : test_cases)
    {
        const hash1024 item = calculate_dataset_item(context, t.index);
        EXPECT_EQ(to_hex(item.hashes[0]), t.hash1_hex) << "index: " << t.index;
        EXPECT_EQ(to_hex(item.hashes[1]), t.hash2_hex) << "index: " << t.index;
    }
}

namespace
{
struct hash_test_case
{
    uint32_t block_number;
    const char* header_hash_hex;
    const char* nonce_hex;
    const char* mix_hash_hex;
    const char* mix_raw_hex;
};

hash_test_case hash_test_cases[] = {
    {2683077, "0313d03c5ed78694c90ecb3d04190b82d5b222c75ba4cab83383dde4d11ed512",
        "8c5eaec000788d41", "00000000000204882a6213f68fe89bc368df25c1ad999f82532a7433e99bc48e",
        "93e85c97b34ccd8091e09ddb513fdc9e680fa8898d4a0737205e60af710a3dcb"}};
}

TEST(ethash, verify_hash_light)
{
    for (const auto& t : hash_test_cases)
    {
        const uint32_t epoch_number = t.block_number / epoch_length;
        const uint64_t nonce = std::stoul(t.nonce_hex, nullptr, 16);
        const hash256 header_hash = to_hash256(t.header_hash_hex);

        epoch_context* context = create_epoch_context(epoch_number);

        hash256 mix = hash_light(*context, header_hash, nonce);
        EXPECT_EQ(to_hex(mix), t.mix_hash_hex);

        destroy_epoch_context(context);
    }
}

TEST(ethash, verify_hash)
{
    for (const auto& t : hash_test_cases)
    {
        const uint32_t epoch_number = t.block_number / epoch_length;
        const uint64_t nonce = std::stoul(t.nonce_hex, nullptr, 16);
        const hash256 header_hash = to_hash256(t.header_hash_hex);

        epoch_context* context = create_epoch_context(epoch_number);
        init_full_dataset(*context);

        hash256 mix = hash(*context, header_hash, nonce);
        EXPECT_EQ(to_hex(mix), t.mix_hash_hex);

        destroy_epoch_context(context);
    }
}

TEST(ethash, small_dataset)
{
    // This test creates an extremely small dataset for full search to discover
    // sync issues between threads.

    constexpr size_t num_treads = 8;
    constexpr size_t num_dataset_items = 501;
    constexpr uint64_t target = uint64_t(1) << 48;

    epoch_context context = create_epoch_context_mock(0);
    context.full_dataset_size = num_dataset_items * sizeof(hash1024);
    init_full_dataset(context);

    std::array<std::future<uint64_t>, num_treads> futures;
    for (auto& f : futures)
        f = std::async(std::launch::async, [&] { return search(context, {}, target, 4892, 30000); });

    for (auto& f : futures)
        EXPECT_EQ(f.get(), 27777);
}

TEST(ethash, small_dataset_light)
{
    constexpr size_t num_dataset_items = 501;
    constexpr uint64_t target = uint64_t(1) << 55;

    epoch_context context = create_epoch_context_mock(0);
    context.full_dataset_size = num_dataset_items * sizeof(hash1024);

    uint64_t solution = search_light(context, {}, target, 4990, 10);
    EXPECT_EQ(solution, 4999);

    solution = search_light(context, {}, target, 5000, 10);
    EXPECT_EQ(solution, 0);
}

TEST(ethash, init_full_dataset_oom)
{
    auto mock_context = create_epoch_context_mock(0);

    constexpr uint64_t max = std::numeric_limits<size_t>::max();
    uint64_t big_size = uint64_t(1) << 40;
    size_t size = static_cast<size_t>(std::min(big_size, max));
    mock_context.full_dataset_size = size;
    EXPECT_FALSE(init_full_dataset(mock_context));
}
