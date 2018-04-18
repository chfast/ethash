// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "ethash-internal.hpp"

#include <memory>
#include <mutex>

#if !defined(__has_cpp_attribute)
#define __has_cpp_attribute(x) 0
#endif

#if __has_cpp_attribute(gnu::noinline)
#define ATTRIBUTE_NOINLINE [[gnu::noinline]]
#elif _MSC_VER
#define ATTRIBUTE_NOINLINE __declspec(noinline)
#else
#define ATTRIBUTE_NOINLINE
#endif

namespace ethash
{
namespace managed
{
namespace
{
std::mutex shared_context_mutex;
std::shared_ptr<ethash_epoch_context> shared_context;
thread_local std::shared_ptr<ethash_epoch_context> thread_local_context;

/// Update thread local epoch context.
///
/// This function is on the slow path. It's separated to allow inlining the fast
/// path.
///
/// @todo: Check noexpect.
ATTRIBUTE_NOINLINE
void update_local_context(int epoch_number)
{
    // Release the shared pointer of the obsoleted context.
    thread_local_context.reset();

    // Local context invalid, check the shared context.
    std::lock_guard<std::mutex> lock{shared_context_mutex};

    if (!shared_context || shared_context->epoch_number != epoch_number)
    {
        // Release the shared pointer of the obsoleted context.
        shared_context.reset();

        // Build new context.
        shared_context = {ethash_create_epoch_context(epoch_number), ethash_destroy_epoch_context};
    }

    thread_local_context = shared_context;
}
}  // namespace

const ethash_epoch_context& get_epoch_context(int epoch_number)
{
    // Check if local context matches epoch number.
    if (!thread_local_context || thread_local_context->epoch_number != epoch_number)
        update_local_context(epoch_number);

    return *thread_local_context;
}

memory_ref get_light_cache_data(int epoch_number)
{
    auto& context = get_epoch_context(epoch_number);
    const uint8_t* const data = reinterpret_cast<uint8_t*>(context.light_cache[0].bytes);
    const size_t size = get_light_cache_size(context.light_cache_num_items);
    return {data, size};
}

result hash(int epoch_number, const hash256& header_hash, uint64_t nonce)
{
    return ethash::hash_light(get_epoch_context(epoch_number), header_hash, nonce);
}

bool verify(int block_number, const hash256& header_hash, const hash256& mix_hash, uint64_t nonce,
    uint64_t target)
{
    return ethash::verify(
        get_epoch_context(get_epoch_number(block_number)), header_hash, mix_hash, nonce, target);
}
}  // namespace managed
}  // namespace ethash
