#pragma once

// #include "../../include/ethash/hash_types.h"

#include "../sha512-256/include/sha512-256-impl.h"
#include "WjCryptLib_Sha512.h"
#include "support/attributes.h"
#include <ethash/hash_types.hpp>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace ethash
{
// void Sha512Calculate(void const* Buffer,  // [in]
//     uint32_t BufferSize,                  // [in]
//     ethash_hash512* Digest                // [in]
// )
// {
//     Sha512Context context;

//     Sha512Initialise(&context);
//     Sha512Update(&context, Buffer, BufferSize);
//     Sha512Finalise(&context, Digest);
// }

inline hash512 ethash_sha512(const hash512& input)
{
    // union ethash_hash512 hash;
    // Sha512Calculate(input.bytes, sizeof(input.bytes), &hash);
    // return hash;
    union ethash_hash512 hash;
    sw::sha512::calculate(input.bytes, sizeof(input.bytes), hash.word64s);
    return hash;
}

inline hash512 ethash_sha512(const hash512& input, size_t size)
{
    size++;
    // union ethash_hash512 hash;
    // Sha512Calculate(input.bytes, sizeof(input.bytes), &hash);
    // return hash;
    union ethash_hash512 hash;
    sw::sha512::calculate(input.bytes, sizeof(input.bytes), hash.word64s);
    return hash;
}

// static hash512 ethash_sha512(const uint8_t* data)
// {
//     // return ethash_sha512(data, sizeof(data));
//     union ethash_hash512 hash;
//     Sha512Calculate(data, sizeof(data), &hash);
//     return hash;
// }

inline hash512 ethash_sha512(const uint8_t* data, size_t size)
{
    union ethash_hash512 hash;
    sw::sha512::calculate(data, size, hash.word64s);
    return hash;
}

inline hash256 ethash_sha512_256(const hash256& input, size_t size)
{
    size++;
    union ethash_hash256 hash;
    sw::sha512_256::calculate(input.bytes, sizeof(input.bytes), hash.word64s);
    return hash;
}

inline hash256 ethash_sha512_256(const hash256& input)
{
    // return ethash_sha512_256(input.bytes);
    union ethash_hash256 hash;
    sw::sha512_256::calculate(input.bytes, sizeof(input.bytes), hash.word64s);
    return hash;
}

// inline hash256 ethash_sha512_256_32(const uint8_t* data)
// {
//     return ethash_sha512_256(data, sizeof(data));
// }

inline hash256 ethash_sha512_256(const uint8_t* data
    , size_t size
)
{
    union ethash_hash256 hash;
    sw::sha512_256::calculate(data, size, hash.word64s);
    return hash;
}

inline hash256 ethash_sha512_256_32(const uint8_t* data
)
{
    union ethash_hash256 hash;
    sw::sha512_256::calculate(data, 32, hash.word64s);
    return hash;
}

// inline hash512 keccak512(const hash512& input) noexcept
// {
//     return ethash_keccak512_64(input.bytes);
// }
}  // namespace ethash
