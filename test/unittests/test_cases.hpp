// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

/// @file
/// Shared test cases.

#pragma once

// Put in anonymous namespace to allow be include in multiple files
// but also make iteration over test cases easy with range-based for loop.
namespace
{
struct hash_test_case
{
    int block_number;
    const char* header_hash_hex;
    const char* nonce_hex;
    const char* mix_hash_hex;
    const char* final_hash_hex;
};

hash_test_case hash_test_cases[] = {
    {2683077, "0313d03c5ed78694c90ecb3d04190b82d5b222c75ba4cab83383dde4d11ed512",
        "8c5eaec000788d41", "93e85c97b34ccd8091e09ddb513fdc9e680fa8898d4a0737205e60af710a3dcb",
        "00000000000204882a6213f68fe89bc368df25c1ad999f82532a7433e99bc48e"},
    {5000000, "bc544c2baba832600013bd5d1983f592e9557d04b0fb5ef7a100434a5fc8d52a",
        "4617a20003ba3f25", "94cd4e844619ee20989578276a0a9046877d569d37ba076bf2e8e34f76189dea",
        "0000000000001a5b18ae31b7417b7c045bc6dd78cc3b694c4bebfaefef6b3c56"},
    {5306861, "53a005f209a4dc013f022a5078c6b38ced76e767a30367ff64725f23ec652a9f",
        "d337f82001e992c5", "26c64f063dac85ae3585526b446be6754faf044876aa3e20f770bea567e04d1d",
        "00000000000003f1554d8071ff0903268fcb70f30f4af3bf7ec7dc69cdf509f3"},
};
}