// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include "ethash-internal.hpp"

namespace ubqhash
{
using namespace ethash;

void build_light_cache(hash512 cache[], int num_items, const hash256& seed) noexcept;
}  // namespace ubqhash