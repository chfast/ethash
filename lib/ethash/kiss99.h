/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * KISS PRNG by the spec from 1999.
 *
 * The implementation of KISS pseudo-random number generator
 * by the specification published on 21 Jan 1999 in
 * http://www.cse.yorku.ca/~oz/marsaglia-rng.html.
 * The KISS is not versioned so here we are using `kiss99` prefix to indicate
 * the version from 1999.
 *
 * The specification uses `unsigned long` type with the intention for 32-bit
 * values. Because in GCC/clang for 64-bit architectures `unsigned long` is
 * 64-bit size type, here the explicit `uint32_t` type is used.
 *
 * @defgroup kiss99 KISS99
 * @{
 */

/**
 * The KISS generator state.
 */
struct kiss99_state
{
    uint32_t z;
    uint32_t w;
    uint32_t jsr;
    uint32_t jcong;
};

/**
 * Creates KISS generator state with default values provided by the specification.
 *
 * Users MAY use other initial values for the state.
 *
 * @return The initialized KISS state.
 */
struct kiss99_state kiss99_init();

/**
 *
 * @param state
 * @return
 */
uint32_t kiss99_generate(struct kiss99_state* state);

/** @} */

#ifdef __cplusplus
}
#endif
