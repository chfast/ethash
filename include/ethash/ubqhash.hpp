// ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <ethash/ethash.hpp>

namespace ubqhash
{
using namespace ethash;

/// Creates Ubqhash epoch context without full dataset.
epoch_context_ptr create_epoch_context(int epoch_number) noexcept;

}  // namespace ubqhash
