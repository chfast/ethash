// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// @file
/// ProgPoW test vectors.

#pragma once

namespace  // In anonymous namespace to allow including in multiple compilation units.
{
/// Defines a test case for ProgPoW hash() function.
struct progpow_hash_test_case
{
    int block_number;
    const char* header_hash_hex;
    const char* nonce_hex;
    const char* mix_hash_hex;
    const char* final_hash_hex;
};

progpow_hash_test_case progpow_hash_test_cases[] = {
    {0, "0000000000000000000000000000000000000000000000000000000000000000", "0000000000000000",
        "a09ffaa0f2b5d47a98c2d4fbc0e90936710dd2b2a220fce04e8d55a6c6a093d6",
        "7ea12cfc33f64616ab7dbbddf3362ee7dd3e1e20d60d860a85c51d6559c912c4"},
    {49, "7ea12cfc33f64616ab7dbbddf3362ee7dd3e1e20d60d860a85c51d6559c912c4", "0000000006ff2c47",
        "4e453d59426905122ef3d176a6fe660f29b53fdf2f82b5af2753dbaaebebf609",
        "f0167e445f8510504ce024856ec614a1a4461610bf58caa32df731ee4c315641"},
    {50, "f0167e445f8510504ce024856ec614a1a4461610bf58caa32df731ee4c315641", "00000000076e482e",
        "4e5291ae6132f64bff00dd05861721b0da701f789e7e65d096b9affa24bffd7e",
        "fdc3bce3e0d0b1a5af43f84acc7d5421d423ec5d3b7e41698178b24c459a6cbe"},
    {99, "fdc3bce3e0d0b1a5af43f84acc7d5421d423ec5d3b7e41698178b24c459a6cbe", "000000003917afab",
        "d35c7e4012204d1db243dc7cf0bf2075f897e362e6ad2b36c02e325cfc6f8dbb",
        "5b014c2c706476b56cf3b9c37ed999d30b20c0fb038d27cc94c991dacef62033"},
    {29950, "5b014c2c706476b56cf3b9c37ed999d30b20c0fb038d27cc94c991dacef62033", "005d409dbc23a62a",
        "0c64704dedb0677149b47fabc6726e9ff0585233692c8562e485a330ce90c0e9",
        "a01b432e82cacaae095ef402b575f1764c45247ba9cf17e99d5432cf00829ee2"},
    {29999, "a01b432e82cacaae095ef402b575f1764c45247ba9cf17e99d5432cf00829ee2", "005db5fa4c2a3d03",
        "3d95cad9cf4513bb31a4766d3a2f488bbff1baa57da8b2252e246ac91594c769",
        "0fc3e6e1392033619f614ec3236d8fbfcefe94d9fdc341a4d7daeffa0b8ad35d"},
    {30000, "0fc3e6e1392033619f614ec3236d8fbfcefe94d9fdc341a4d7daeffa0b8ad35d", "005db8607994ff30",
        "7ee9d0c571ed35073404454eebe9a73a6d677a32446cf6c427ee63a63bd512da",
        "b94de4495555dc2ab4ad8725cabd395178813c8c434134b2f25062b5f72dafb9"},
    {30049, "b94de4495555dc2ab4ad8725cabd395178813c8c434134b2f25062b5f72dafb9", "005e2e215a8ca2e7",
        "7a16d37208288152237afdc13724d26fe7aadf3cd354a42c587a4192761ef18e",
        "e152d3770855cea35a94ee53ab321f93ee3a426513c6ab1ec5e8d81ea9a661d7"},
    {30050, "e152d3770855cea35a94ee53ab321f93ee3a426513c6ab1ec5e8d81ea9a661d7", "005e30899481055e",
        "005df2434f2a5265c2ed0d13dd12308795620202d2784a40967461c383f859a3",
        "55d013e85571e46e914a7529909fbfc686965a92c7baaef2e89e5b5f533a6dc9"},
    {30099, "55d013e85571e46e914a7529909fbfc686965a92c7baaef2e89e5b5f533a6dc9", "005ea6aef136f88b",
        "d8b1046cc2c8273a06e6f7ce19b7b4aefb7fb43b141721663252e2872b654548",
        "8ba5629b6affa0514c2f4951c3a63761465ef0e5be7cbb8f9ce230a5564faccb"},
    {59950, "8ba5629b6affa0514c2f4951c3a63761465ef0e5be7cbb8f9ce230a5564faccb", "02ebe0503bd7b1da",
        "b3131de1a747449e5328f50742447d5c6da637a5d141a117caf9a986bd524de9",
        "10af438404304f4a7de0b07e7d08bfc80b521860237e3e2d47f77630eef5f742"},
    {59999, "10af438404304f4a7de0b07e7d08bfc80b521860237e3e2d47f77630eef5f742", "02edb6275bd221e3",
        "87f7d6c73fb86a5ed00d2ad7fff7b2a8a9796c3138b31f2473b89065946cb0ed",
        "3863e5c767a6b0d28f5cf1d261e35c52fe03f7fd690d50c10596ec73d7595887"},
};
}  // namespace
