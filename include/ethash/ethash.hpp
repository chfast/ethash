// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

///@file
///
/// API design decisions:
///
/// 1. Signed integer type is used whenever the size of the type is not
///    restricted by the Ethash specification.
///    See http://www.aristeia.com/Papers/C++ReportColumns/sep95.pdf.
///    See https://stackoverflow.com/questions/10168079/why-is-size-t-unsigned/.
///    See https://github.com/Microsoft/GSL/issues/171.

#pragma once

#include <ethash/ethash.h>

#include <cstdint>
#include <cstring>

namespace ethash
{
static constexpr int epoch_length = 30000;


union hash256
{
    uint64_t words[4] = {
        0,
    };
    uint32_t hwords[8];
    char bytes[32];

    /// Named constructor from bytes.
    ///
    /// @param bytes  A pointer to array of at least 32 bytes.
    /// @return       The constructed hash.
    static hash256 from_bytes(const uint8_t bytes[32]) noexcept
    {
        hash256 h;
        std::memcpy(&h, bytes, sizeof(h));
        return h;
    }
};

struct result
{
    hash256 final_hash;
    hash256 mix_hash;
};


/// Calculates the epoch number out of the block number.
inline int get_epoch_number(int block_number)
{
    return block_number / epoch_length;
}


/// Init full dataset in the epoch context.
///
/// This allocates the memory for the full dataset and inits dataset items
/// marked as "not-generated".
///
/// @param context  The epoch context.
/// @return  Returns true if memory allocations succeeded, false otherwise.
bool init_full_dataset(ethash_epoch_context& context) noexcept;


result hash_light(const ethash_epoch_context& context, const hash256& header_hash, uint64_t nonce);

result hash(const ethash_epoch_context& context, const hash256& header_hash, uint64_t nonce);

bool verify(const ethash_epoch_context& context, const hash256& header_hash,
    const hash256& mix_hash, uint64_t nonce, uint64_t target);

uint64_t search_light(const ethash_epoch_context& context, const hash256& header_hash,
    uint64_t target, uint64_t start_nonce, size_t iterations);

uint64_t search(const ethash_epoch_context& context, const hash256& header_hash, uint64_t target,
    uint64_t start_nonce, size_t iterations);


/// Tries to find the epoch number matching the given seed hash.
///
/// Mining pool protocols (many variants of stratum and "getwork") send out
/// seed hash instead of epoch number to workers. This function tries to recover
/// the epoch number from this seed hash.
///
/// @param seed  Ethash seed hash.
/// @return      The epoch number or -1 if not found.
int find_epoch_number(const hash256& seed) noexcept;


namespace managed
{

/// Compute Ethash hash using light cache and the shared epoch context managed by the library.
result hash(int epoch_number, const hash256& header_hash, uint64_t nonce);

bool verify(int block_number, const hash256& header_hash, const hash256& mix_hash, uint64_t nonce,
    uint64_t target);
}
}
