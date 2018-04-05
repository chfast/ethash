// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include <ethash/keccak.hpp>

#include "helpers.hpp"

#include <gtest/gtest.h>

using namespace ethash;

static const char keccak256_of_empty[] =
    "\xc5\xd2\x46\x01\x86\xf7\x23\x3c\x92\x7e\x7d\xb2\xdc\xc7\x03\xc0\xe5\x00\xb6\x53\xca\x82\x27"
    "\x3b\x7b\xfa\xd8\x04\x5d\x85\xa4\x70";

static const char keccack512_of_empty[] =
    "\x0e\xab\x42\xde\x4c\x3c\xeb\x92\x35\xfc\x91\xac\xff\xe7\x46\xb2\x9c\x29\xa8\xc3\x66\xb7\xc6"
    "\x0e\x4e\x67\xc4\x66\xf3\x6a\x43\x04\xc0\x0f\xa9\xca\xf9\xd8\x79\x76\xba\x46\x9b\xcb\xe0\x67"
    "\x13\xb4\x35\xf0\x91\xef\x27\x69\xfb\x16\x0c\xda\xb3\x3d\x36\x70\x68\x0e";


TEST(keccak, empty_256)
{
    uint64_t dummy_byte = 0xd;
    hash256 h = keccak<256>(&dummy_byte, 0);
    std::string strh{h.bytes, sizeof(h)};
    EXPECT_EQ(strh, std::string(keccak256_of_empty, sizeof(h)));
}

TEST(keccak, hello_world_256)
{
    std::string hello_world{"Hello World! "};
    for (int i = 0; i < 4; ++i)
        hello_world += hello_world;
    ASSERT_GT(hello_world.size(), 200);
    hash256 h = keccak<256>((uint64_t*)hello_world.data(), hello_world.size() / 8);
    EXPECT_EQ(to_hex(h), "0d660e6953b0fbbd6476864d4474b2f1aa943358cd4d723af30110f6c465faaf");
}

TEST(keccak, empty_512)
{
    uint64_t dummy_byte = 0xd;
    hash512 h = keccak<512>(&dummy_byte, 0);
    std::string strh{h.bytes, sizeof(h)};
    EXPECT_EQ(strh, std::string(keccack512_of_empty, sizeof(h)));
}

TEST(keccak, hello_world_512)
{
    std::string hello_world{"Hello World! "};
    for (int i = 0; i < 4; ++i)
        hello_world += hello_world;
    ASSERT_GT(hello_world.size(), 200);
    hash512 h = keccak<512>((uint64_t*)hello_world.data(), hello_world.size() / 8);
    EXPECT_EQ(to_hex(h),
        "f67f9221f3a039937b7f0ce58b48ef68e71a7d8b381a3476bf515716b919a5440bf8107127e02413d156317a9e"
        "9f42bc5dea973247f1a0a2e1045cb3fbe8db72");
}

TEST(keccak, double512)
{
    hash1024 input = {};
    input.bytes[0] = 126;
    input.bytes[127] = 127;
    hash1024 output = double_keccak(input);
    EXPECT_EQ(to_hex(output.hashes[0]),
        "7e62a25d706e921aaec4d4916c8822126f4470bd6d4d802e1f288d3b89356fee0e13907a71809810566cf71ae8"
        "25306fc3197a83e6d315a04e61f28029799a98");
    EXPECT_EQ(to_hex(output.hashes[1]),
        "67525363f9cfa5b0ee6553a542b335a1d47048e4e614960537f737e5f951d162504ee77f6f76e16ea19b2a9fb8"
        "3e5056e6ace516f9f488e072e65bb997a6de02");
}

TEST(helpers, to_hex)
{
    hash256 h;
    h.bytes[0] = 0;
    h.bytes[1] = 1;
    h.bytes[2] = 2;
    h.bytes[3] = 3;
    h.bytes[4] = 4;
    h.bytes[5] = 5;
    h.bytes[6] = 6;
    h.bytes[7] = 7;
    h.bytes[8] = 8;
    h.bytes[9] = 9;
    h.bytes[10] = 10;

    h.bytes[31] = -1;

    auto s = to_hex(h);
    EXPECT_EQ(s, "000102030405060708090a0000000000000000000000000000000000000000ff");
}

TEST(helpers, to_hash256)
{
    const char* hex = "0313d03c5ed78694c90ecb3d04190b82d5b222c75ba4cab83383dde4d11ed512";
    hash256 h = to_hash256(hex);
    std::string s = to_hex(h);
    EXPECT_EQ(s, hex);
}