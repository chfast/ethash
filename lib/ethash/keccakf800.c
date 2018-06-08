/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#include <stdint.h>

 /* Implementation based on:
	https://github.com/mjosaarinen/tiny_sha3/blob/master/sha3.c
	converted from 64->32 bit words*/
const uint32_t keccakf_rndc[24] = {
	0x00000001, 0x00008082, 0x0000808a, 0x80008000, 0x0000808b, 0x80000001,
	0x80008081, 0x00008009, 0x0000008a, 0x00000088, 0x80008009, 0x8000000a,
	0x8000808b, 0x0000008b, 0x00008089, 0x00008003, 0x00008002, 0x00000080,
	0x0000800a, 0x8000000a, 0x80008081, 0x00008080, 0x80000001, 0x80008008
};

#define ROTL(x,n,w) (((x) << (n)) | ((x) >> ((w) - (n))))
#define ROTL32(x,n) ROTL(x,n,32)	/* 32 bits word */

void keccak_f800_round(uint32_t st[25], const int r)
{

	const uint32_t keccakf_rotc[24] = {
		1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
		27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
	};
	const uint32_t keccakf_piln[24] = {
		10, 7,  11, 17, 18, 3, 5,  16, 8,  21, 24, 4,
		15, 23, 19, 13, 12, 2, 20, 14, 22, 9,  6,  1
	};

	uint32_t t, bc[5];
	/* Theta*/
	uint32_t i = 0, j = 0;
	for (i = 0; i < 5; i++)
		bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

	i = 0;
	for (i = 0; i < 5; i++) {
		t = bc[(i + 4) % 5] ^ ROTL32(bc[(i + 1) % 5], 1);
		j = 0;
		for (j = 0; j < 25; j += 5)
			st[j + i] ^= t;
	}

	/*Rho Pi*/
	i = 0;
	t = st[1];
	for (i = 0; i < 24; i++) {
		uint32_t jj = keccakf_piln[i];
		bc[0] = st[jj];
		st[jj] = ROTL32(t, keccakf_rotc[i]);
		t = bc[0];
	}

	/* Chi*/
	j = 0;
	for (j = 0; j < 25; j += 5) {
		i = 0;
		for (i = 0; i < 5; i++)
			bc[i] = st[j + i];
		i = 0;
		for (i = 0; i < 5; i++)
			st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
	}

	/* Iota*/
	st[0] ^= keccakf_rndc[r];
}
