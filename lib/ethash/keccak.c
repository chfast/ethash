// Copyright 2018 Pawel Bylica.
// Licensed under the Apache License, Version 2.0. See the LICENSE file.
//
// The base implementation of keccakf() taken from libkeccak-tiny
// (https://github.com/coruus/keccak-tiny, CC0).

#include <stdint.h>

static const uint8_t rho[24] = {
    1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44};

static const uint8_t pi[24] = {
    10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1};

static const uint64_t RC[24] = {0x1, 0x8082, 0x800000000000808a, 0x8000000080008000, 0x808b,
    0x80000001, 0x8000000080008081, 0x8000000000008009, 0x8a, 0x88, 0x80008009, 0x8000000a,
    0x8000808b, 0x800000000000008b, 0x8000000000008089, 0x8000000000008003, 0x8000000000008002,
    0x8000000000000080, 0x800a, 0x800000008000000a, 0x8000000080008081, 0x8000000000008080,
    0x80000001, 0x8000000080008008};


static inline uint64_t rol(uint64_t x, unsigned s)
{
    return (x << s) | (x >> (64 - s));
}

#define REPEAT6(e) e e e e e e
#define REPEAT24(e) REPEAT6(e e e e)
#define REPEAT5(e) e e e e e
#define FOR5(index, step, expr) \
    index = 0;                  \
    REPEAT5(expr; (index) += (step);)

void ethash_keccakf(uint64_t* a)
{
    uint64_t b[5] = {};

    for (int i = 0; i < 24; ++i)
    {
        int x;
        int y;

        // Theta:
        FOR5(x, 1, b[x] = 0; FOR5(y, 5, b[x] ^= a[x + y];))
        FOR5(x, 1, FOR5(y, 5, a[y + x] ^= b[(x + 4) % 5] ^ rol(b[(x + 1) % 5], 1);))

        // Rho and pi:
        uint64_t t = a[1];
        x = 0;
        REPEAT24(b[0] = a[pi[x]]; a[pi[x]] = rol(t, rho[x]); t = b[0]; x++;)

        // Chi:
        FOR5(y, 5,
            FOR5(x, 1, b[x] = a[y + x];)
                FOR5(x, 1, a[y + x] = b[x] ^ ((~b[(x + 1) % 5]) & b[(x + 2) % 5]);))

        // Iota:
        a[0] ^= RC[i];
    }
}
