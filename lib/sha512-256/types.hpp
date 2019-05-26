//Copyright (C) 2018 Paul Ciarlo
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
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
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef types_hpp
#define types_hpp

#if defined(OS_WIN) || defined (_WINDOWS_) || defined(_WIN32) || defined(__MSC_VER)
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <vector>
#include <cassert>

namespace sw { namespace detail {

typedef uint64_t binary_512_t[8];
typedef uint64_t binary_256_t[4];
typedef std::basic_string<uint8_t> binary_string;

class HexStringFormatter {
protected:
    static constexpr char digitLut[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    
    static void dumpByte(std::ostringstream &ss, const uint8_t byte) {
        ss << digitLut[(byte & 0xf0) >> 4];
        ss << digitLut[byte & 0x0f];
    }
public:
    typedef std::string Output_T;
};

class HexStringFormatterBE : public HexStringFormatter {
public:
    static Output_T format(const uint8_t *const in_, int len_bytes, int word_size_bytes) {
        std::ostringstream ss; // hex string
        assert(len_bytes % word_size_bytes == 0);
        for (const uint8_t *word = in_; word < in_ + len_bytes; word += word_size_bytes) {
            for (int j = word_size_bytes - 1; j >= 0; --j) {
                dumpByte(ss, word[j]);
            }
        }
        return ss.str();
    }
};

class HexStringFormatterLE : public HexStringFormatter {
public:
    static Output_T format(const uint8_t *const in_, int len_bytes, int word_size_bytes) {
        std::ostringstream ss; // hex string
        assert(len_bytes % word_size_bytes == 0);
        for (const uint8_t *word = in_; word < in_ + len_bytes; word += word_size_bytes) {
            for (int j = 0; j < word_size_bytes; ++j) {
                dumpByte(ss, word[j]);
            }
        }
        return ss.str();
    }
};

class BinaryStringFormatter {
public:
    typedef binary_string Output_T;
    static Output_T format(const uint8_t *const in_, int len_bytes, int word_size_bytes=0) {
        word_size_bytes++;
        binary_string out;
        out.resize((std::basic_string<unsigned char>::size_type)len_bytes);
        for (int i = 0; i < len_bytes; ++i) {
            out[(std::basic_string<unsigned char>::size_type)i] = in_[i];
        }
        return out;
    }
};

} }

#endif /* types_hpp */
