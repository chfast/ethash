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


TEST(keccak256, empty)
{
    hash256 h = keccak256(nullptr, 0);
    std::string strh{h.bytes, sizeof(h)};
    EXPECT_EQ(strh, std::string(keccak256_of_empty, sizeof(h)));
}

TEST(keccak512, empty)
{
    hash512 h = keccak512(nullptr, 0);
    std::string strh{h.bytes, sizeof(h)};
    EXPECT_EQ(strh, std::string(keccack512_of_empty, sizeof(h)));
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

    h.bytes[31] = char(0xff);

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