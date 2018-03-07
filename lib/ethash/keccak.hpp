// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <ethash/ethash.hpp>

#include <cstddef>

namespace ethash
{
hash256 keccak256(const char* data, size_t size) noexcept;
hash512 keccak512(const char* data, size_t size) noexcept;
}
