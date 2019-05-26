// Copyright (C) 2018 Paul Ciarlo
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//* Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//* Neither the name of the copyright holder nor the names of its
//  contributors may be used to endorse or promote products derived from
//  this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef sha512_256_h
#define sha512_256_h

#include "sha512.h"

namespace sw
{
namespace detail
{
/*
Table 3: SHA-512/256 Proposed Initial Constants
   PrimeNumber PrimeValue The first 64bits of the fractional part of the square roots of the primes
h0      17         59         ae5f9156e7b6d99b
h1      18         61         cf6c85d39d1a1e15
h2      19         67         2f73477d6a4563ca
h3      20         71         6d1826cafd82e1ed
h4      21         73         8b43d4570a51b936
h5      22         79         e360b596dc380c3f
h6      23         83         1c456002ce13e9f8
h7      24         89         6f19633143a0af0e
*/
template <typename Formatter_T>
class basic_sha512_256 : public basic_sha512<Formatter_T>
{
public:
    typedef typename Formatter_T::Output_T Output_T;

    basic_sha512_256() : sw::sha512() { sw::sha512::clear(initial_); }
    /**
     * Clear/reset all internal buffers and states.
     */
    static Output_T calculate(const void* data, size_t size)
    {
        basic_sha512_256 r;
        r.update(data, size);
        return r.final_data();
    }

    static void calculate(const void* data, size_t size, uint64_t* out)
    {
        basic_sha512_256 r;
        r.update(data, size);
        return r.final_data(out);
    }

    static Output_T calculate512(const void* data, size_t size)
    {
        basic_sha512_256 r;
        r.update(data, size);
        return r.final_data(512);
    }

    Output_T final_data(int out_bits = 256)
    {
        binary_512_t raw;
        basic_sha512<Formatter_T>::final_data_raw(raw);
        return Formatter_T::format((uint8_t*)raw, out_bits / 8, 64 / 8);
    }

    void final_data(uint64_t* out, int out_bits = 256)
    {
        binary_512_t raw;
        basic_sha512<Formatter_T>::final_data_raw(raw);
        std::memcpy(out, raw, (size_t)out_bits / 8);
    }

private:
    static const uint64_t initial_[8];
};

template <typename CT>
const uint64_t basic_sha512_256<CT>::initial_[8] = {0xae5f9156e7b6d99b, 0xcf6c85d39d1a1e15,
    0x2f73477d6a4563ca, 0x6d1826cafd82e1ed, 0x8b43d4570a51b936, 0xe360b596dc380c3f,
    0x1c456002ce13e9f8, 0x6f19633143a0af0e};

}  // namespace detail
}  // namespace sw

namespace sw
{
typedef detail::basic_sha512_256<detail::HexStringFormatterBE> sha512_256;
}

#endif /* sha512_256_h */
