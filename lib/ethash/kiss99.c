/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#include "kiss99.h"
#include "support/attributes.h"

struct kiss99_state kiss99_init()
{
    struct kiss99_state state = {362436069, 521288629, 123456789, 380116160};
    return state;
}

NO_SANITIZE("unsigned-integer-overflow")
uint32_t kiss99_generate(struct kiss99_state* state)
{
    state->z = 36969 * (state->z & 0xffff) + (state->z >> 16);
    state->w = 18000 * (state->w & 0xffff) + (state->w >> 16);

    state->jcong = 69069 * state->jcong + 1234567;

    state->jsr ^= (state->jsr << 17);
    state->jsr ^= (state->jsr >> 13);
    state->jsr ^= (state->jsr << 5);

    return (((state->z << 16) + state->w) ^ state->jcong) + state->jsr;
}
