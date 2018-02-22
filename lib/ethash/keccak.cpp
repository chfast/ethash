// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "keccak.hpp"

// Declarations for keccak-tiny:

extern "C" int sha3_256(uint8_t* out, size_t outlen, const uint8_t* in, size_t inlen);
extern "C" int sha3_512(uint8_t* out, size_t outlen, const uint8_t* in, size_t inlen);

namespace ethash
{
hash256 keccak256(const char* data, size_t size) noexcept
{
    hash256 h;
    sha3_256(reinterpret_cast<uint8_t*>(h.bytes), sizeof(h), reinterpret_cast<const uint8_t*>(data),
        size);
    return h;
}

hash512 keccak512(const char* data, size_t size) noexcept
{
    hash512 h;
    sha3_512(reinterpret_cast<uint8_t*>(h.bytes), sizeof(h), reinterpret_cast<const uint8_t*>(data),
        size);
    return h;
}
}