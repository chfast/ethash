// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#include "ethash-internal.hpp"

#include <memory>
#include <mutex>

namespace ethash
{
namespace managed
{
namespace
{
std::mutex shared_context_mutex;
std::shared_ptr<ethash_epoch_context> shared_context;
thread_local std::shared_ptr<ethash_epoch_context> thread_local_context;
}  // namespace

std::shared_ptr<ethash_epoch_context> get_epoch_context(int epoch_number)
{
    // Check if local context matches epoch number.
    if (!thread_local_context || thread_local_context->epoch_number != epoch_number)
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
            shared_context = {
                ethash_create_epoch_context(epoch_number), ethash_destroy_epoch_context};
        }

        thread_local_context = shared_context;
    }

    return thread_local_context;
}

result hash(int epoch_number, const hash256& header_hash, uint64_t nonce)
{
    std::shared_ptr<ethash_epoch_context> epoch_context = get_epoch_context(epoch_number);
    return ethash::hash_light(*epoch_context, header_hash, nonce);
}

bool verify(int block_number, const hash256& header_hash, const hash256& mix_hash, uint64_t nonce,
    uint64_t target)
{
    std::shared_ptr<ethash_epoch_context> epoch_context =
        get_epoch_context(get_epoch_number(block_number));
    return ethash::verify(*epoch_context, header_hash, mix_hash, nonce, target);
}
}  // namespace managed
}  // namespace ethash
