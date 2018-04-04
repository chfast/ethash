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
std::mutex cache_building_mutex;
std::shared_ptr<ethash_epoch_context> current_context;

std::shared_ptr<ethash_epoch_context> get_epoch_context(int epoch_number)
{
    std::lock_guard<std::mutex> lock{cache_building_mutex};

    if (!current_context || current_context->epoch_number != epoch_number)
        current_context = {ethash_create_epoch_context(epoch_number), ethash_destroy_epoch_context};

    return current_context;
}
}

bool verify(int block_number, const hash256& header_hash, const hash256& mix_hash, uint64_t nonce,
    uint64_t target)
{
    std::shared_ptr<ethash_epoch_context> epoch_context =
        get_epoch_context(get_epoch_number(block_number));
    return ethash::verify(*epoch_context, header_hash, mix_hash, nonce, target);
}
}
}
