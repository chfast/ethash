// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.

#pragma once

#include <ethash/types.hpp>

#include <string>

template<typename Hash>
inline std::string to_hex(const Hash& h)
{
    static const auto hex_chars = "0123456789abcdef";
    std::string str;
    str.reserve(sizeof(h) * 2);
    for (auto b: h.bytes)
    {
        str.push_back(hex_chars[uint8_t(b) >> 4]);
        str.push_back(hex_chars[uint8_t(b) & 0xf]);
    }
    return str;
}
