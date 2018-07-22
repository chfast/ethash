/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#include <stdint.h>
#include <string.h>

/*   math_ops.hpp     */

//#define ROTR32(x, n)  (((0U + (x)) << (32 - (n))) | ((x) >> (n)))  // Assumes that x is uint32_t and 0 < n < 32

#define ROTL(x,n,w) (((x) << (n)) | ((x) >> ((w) - (n))))
#define ROTL32(x,n) ROTL(x,n,32)        /* 32 bits word */

#define ROTR(x,n,w) (((x) >> (n)) | ((x) << ((w) - (n))))

#define ROTR32(x,n) ROTR(x,n,32)        /* 32 bits word */

#define min_(a,b) ((a<b) ? a : b)
//#define mul_hi(a, b) __umulhi(a, b)
static
uint32_t mul_hi (uint32_t a, uint32_t b){
    uint64_t result = (uint64_t) a * (uint64_t) b;
    return  (uint32_t) (result>>32);
}
//#define clz(a) __clz(a)
static
uint32_t clz (uint32_t a){
    uint32_t result = 0;
    for(int i=31;i>=0;i--){
        if(((a>>i)&1) == 0)
            result ++;
        else
            break;
    }
    return result;
}
//#define popcount(a) __popc(a)
static
uint32_t popcount(uint32_t a) {
	uint32_t result = 0;
	for (int i = 31; i >= 0; i--) {
		if (((a >> i) & 1) == 1)
			result++;
	}
	return result;
}

/********** keccakf1600.c ***********************/
/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#include <stdint.h>

static uint64_t rol(uint64_t x, unsigned s)
{
    return (x << s) | (x >> (64 - s));
}

static const uint64_t round_constants[24] = {
    0x0000000000000001,
    0x0000000000008082,
    0x800000000000808a,
    0x8000000080008000,
    0x000000000000808b,
    0x0000000080000001,
    0x8000000080008081,
    0x8000000000008009,
    0x000000000000008a,
    0x0000000000000088,
    0x0000000080008009,
    0x000000008000000a,
    0x000000008000808b,
    0x800000000000008b,
    0x8000000000008089,
    0x8000000000008003,
    0x8000000000008002,
    0x8000000000000080,
    0x000000000000800a,
    0x800000008000000a,
    0x8000000080008081,
    0x8000000000008080,
    0x0000000080000001,
    0x8000000080008008,
};

static
void ethash_keccakf1600(uint64_t state[25])
{
    /* The implementation based on the "simple" implementation by Ronny Van Keer. */

    int round;

    uint64_t Aba, Abe, Abi, Abo, Abu;
    uint64_t Aga, Age, Agi, Ago, Agu;
    uint64_t Aka, Ake, Aki, Ako, Aku;
    uint64_t Ama, Ame, Ami, Amo, Amu;
    uint64_t Asa, Ase, Asi, Aso, Asu;

    uint64_t Eba, Ebe, Ebi, Ebo, Ebu;
    uint64_t Ega, Ege, Egi, Ego, Egu;
    uint64_t Eka, Eke, Eki, Eko, Eku;
    uint64_t Ema, Eme, Emi, Emo, Emu;
    uint64_t Esa, Ese, Esi, Eso, Esu;

    uint64_t Ba, Be, Bi, Bo, Bu;

    uint64_t Da, De, Di, Do, Du;

    Aba = state[0];
    Abe = state[1];
    Abi = state[2];
    Abo = state[3];
    Abu = state[4];
    Aga = state[5];
    Age = state[6];
    Agi = state[7];
    Ago = state[8];
    Agu = state[9];
    Aka = state[10];
    Ake = state[11];
    Aki = state[12];
    Ako = state[13];
    Aku = state[14];
    Ama = state[15];
    Ame = state[16];
    Ami = state[17];
    Amo = state[18];
    Amu = state[19];
    Asa = state[20];
    Ase = state[21];
    Asi = state[22];
    Aso = state[23];
    Asu = state[24];

    for (round = 0; round < 24; round += 2)
    {
        /* Round (round + 0): Axx -> Exx */

        Ba = Aba ^ Aga ^ Aka ^ Ama ^ Asa;
        Be = Abe ^ Age ^ Ake ^ Ame ^ Ase;
        Bi = Abi ^ Agi ^ Aki ^ Ami ^ Asi;
        Bo = Abo ^ Ago ^ Ako ^ Amo ^ Aso;
        Bu = Abu ^ Agu ^ Aku ^ Amu ^ Asu;

        Da = Bu ^ rol(Be, 1);
        De = Ba ^ rol(Bi, 1);
        Di = Be ^ rol(Bo, 1);
        Do = Bi ^ rol(Bu, 1);
        Du = Bo ^ rol(Ba, 1);

        Ba = Aba ^ Da;
        Be = rol(Age ^ De, 44);
        Bi = rol(Aki ^ Di, 43);
        Bo = rol(Amo ^ Do, 21);
        Bu = rol(Asu ^ Du, 14);
        Eba = Ba ^ (~Be & Bi) ^ round_constants[round];
        Ebe = Be ^ (~Bi & Bo);
        Ebi = Bi ^ (~Bo & Bu);
        Ebo = Bo ^ (~Bu & Ba);
        Ebu = Bu ^ (~Ba & Be);

        Ba = rol(Abo ^ Do, 28);
        Be = rol(Agu ^ Du, 20);
        Bi = rol(Aka ^ Da, 3);
        Bo = rol(Ame ^ De, 45);
        Bu = rol(Asi ^ Di, 61);
        Ega = Ba ^ (~Be & Bi);
        Ege = Be ^ (~Bi & Bo);
        Egi = Bi ^ (~Bo & Bu);
        Ego = Bo ^ (~Bu & Ba);
        Egu = Bu ^ (~Ba & Be);

        Ba = rol(Abe ^ De, 1);
        Be = rol(Agi ^ Di, 6);
        Bi = rol(Ako ^ Do, 25);
        Bo = rol(Amu ^ Du, 8);
        Bu = rol(Asa ^ Da, 18);
        Eka = Ba ^ (~Be & Bi);
        Eke = Be ^ (~Bi & Bo);
        Eki = Bi ^ (~Bo & Bu);
        Eko = Bo ^ (~Bu & Ba);
        Eku = Bu ^ (~Ba & Be);

        Ba = rol(Abu ^ Du, 27);
        Be = rol(Aga ^ Da, 36);
        Bi = rol(Ake ^ De, 10);
        Bo = rol(Ami ^ Di, 15);
        Bu = rol(Aso ^ Do, 56);
        Ema = Ba ^ (~Be & Bi);
        Eme = Be ^ (~Bi & Bo);
        Emi = Bi ^ (~Bo & Bu);
        Emo = Bo ^ (~Bu & Ba);
        Emu = Bu ^ (~Ba & Be);

        Ba = rol(Abi ^ Di, 62);
        Be = rol(Ago ^ Do, 55);
        Bi = rol(Aku ^ Du, 39);
        Bo = rol(Ama ^ Da, 41);
        Bu = rol(Ase ^ De, 2);
        Esa = Ba ^ (~Be & Bi);
        Ese = Be ^ (~Bi & Bo);
        Esi = Bi ^ (~Bo & Bu);
        Eso = Bo ^ (~Bu & Ba);
        Esu = Bu ^ (~Ba & Be);


        /* Round (round + 1): Exx -> Axx */

        Ba = Eba ^ Ega ^ Eka ^ Ema ^ Esa;
        Be = Ebe ^ Ege ^ Eke ^ Eme ^ Ese;
        Bi = Ebi ^ Egi ^ Eki ^ Emi ^ Esi;
        Bo = Ebo ^ Ego ^ Eko ^ Emo ^ Eso;
        Bu = Ebu ^ Egu ^ Eku ^ Emu ^ Esu;

        Da = Bu ^ rol(Be, 1);
        De = Ba ^ rol(Bi, 1);
        Di = Be ^ rol(Bo, 1);
        Do = Bi ^ rol(Bu, 1);
        Du = Bo ^ rol(Ba, 1);

        Ba = Eba ^ Da;
        Be = rol(Ege ^ De, 44);
        Bi = rol(Eki ^ Di, 43);
        Bo = rol(Emo ^ Do, 21);
        Bu = rol(Esu ^ Du, 14);
        Aba = Ba ^ (~Be & Bi) ^ round_constants[round + 1];
        Abe = Be ^ (~Bi & Bo);
        Abi = Bi ^ (~Bo & Bu);
        Abo = Bo ^ (~Bu & Ba);
        Abu = Bu ^ (~Ba & Be);

        Ba = rol(Ebo ^ Do, 28);
        Be = rol(Egu ^ Du, 20);
        Bi = rol(Eka ^ Da, 3);
        Bo = rol(Eme ^ De, 45);
        Bu = rol(Esi ^ Di, 61);
        Aga = Ba ^ (~Be & Bi);
        Age = Be ^ (~Bi & Bo);
        Agi = Bi ^ (~Bo & Bu);
        Ago = Bo ^ (~Bu & Ba);
        Agu = Bu ^ (~Ba & Be);

        Ba = rol(Ebe ^ De, 1);
        Be = rol(Egi ^ Di, 6);
        Bi = rol(Eko ^ Do, 25);
        Bo = rol(Emu ^ Du, 8);
        Bu = rol(Esa ^ Da, 18);
        Aka = Ba ^ (~Be & Bi);
        Ake = Be ^ (~Bi & Bo);
        Aki = Bi ^ (~Bo & Bu);
        Ako = Bo ^ (~Bu & Ba);
        Aku = Bu ^ (~Ba & Be);

        Ba = rol(Ebu ^ Du, 27);
        Be = rol(Ega ^ Da, 36);
        Bi = rol(Eke ^ De, 10);
        Bo = rol(Emi ^ Di, 15);
        Bu = rol(Eso ^ Do, 56);
        Ama = Ba ^ (~Be & Bi);
        Ame = Be ^ (~Bi & Bo);
        Ami = Bi ^ (~Bo & Bu);
        Amo = Bo ^ (~Bu & Ba);
        Amu = Bu ^ (~Ba & Be);

        Ba = rol(Ebi ^ Di, 62);
        Be = rol(Ego ^ Do, 55);
        Bi = rol(Eku ^ Du, 39);
        Bo = rol(Ema ^ Da, 41);
        Bu = rol(Ese ^ De, 2);
        Asa = Ba ^ (~Be & Bi);
        Ase = Be ^ (~Bi & Bo);
        Asi = Bi ^ (~Bo & Bu);
        Aso = Bo ^ (~Bu & Ba);
        Asu = Bu ^ (~Ba & Be);
    }

    state[0] = Aba;
    state[1] = Abe;
    state[2] = Abi;
    state[3] = Abo;
    state[4] = Abu;
    state[5] = Aga;
    state[6] = Age;
    state[7] = Agi;
    state[8] = Ago;
    state[9] = Agu;
    state[10] = Aka;
    state[11] = Ake;
    state[12] = Aki;
    state[13] = Ako;
    state[14] = Aku;
    state[15] = Ama;
    state[16] = Ame;
    state[17] = Ami;
    state[18] = Amo;
    state[19] = Amu;
    state[20] = Asa;
    state[21] = Ase;
    state[22] = Asi;
    state[23] = Aso;
    state[24] = Asu;
}

/********************* keccak.c *******************/
#define to_le64(X) X
/** Loads 64-bit integer from given memory location as little-endian number. */
static uint64_t load_le(const uint8_t* data)
{
    /* memcpy is the best way of expressing the intention. Every compiler will
       optimize is to single load instruction if the target architecture
       supports unaligned memory access (GCC and clang even in O0).
       This is great trick because we are violating C/C++ memory alignment
       restrictions with no performance penalty. */
    uint64_t word;
    memcpy(&word, data, sizeof(word));
    return to_le64(word);
}

static void keccak(
    uint64_t* out, size_t bits, const uint8_t* data, size_t size)
{
    static const size_t word_size = sizeof(uint64_t);
    const size_t hash_size = bits / 8;
    const size_t block_size = (1600 - bits * 2) / 8;

    size_t i;
    uint64_t* state_iter;
    uint64_t last_word = 0;
    uint8_t* last_word_iter = (uint8_t*)&last_word;

    uint64_t state[25] = {0};

    while (size >= block_size)
    {
        for (i = 0; i < (block_size / word_size); ++i)
        {
            state[i] ^= load_le(data);
            data += word_size;
        }

        ethash_keccakf1600(state);

        size -= block_size;
    }

    state_iter = state;

    while (size >= word_size)
    {
        *state_iter ^= load_le(data);
        ++state_iter;
        data += word_size;
        size -= word_size;
    }

    while (size > 0)
    {
        *last_word_iter = *data;
        ++last_word_iter;
        ++data;
        --size;
    }
    *last_word_iter = 0x01;
    *state_iter ^= to_le64(last_word);

    state[(block_size / word_size) - 1] ^= 0x8000000000000000;

    ethash_keccakf1600(state);

    for (i = 0; i < (hash_size / word_size); ++i)
        out[i] = to_le64(state[i]);
}

/* origin keccakf800.c */
 /* Implementation based on:
	https://github.com/mjosaarinen/tiny_sha3/blob/master/sha3.c
	converted from 64->32 bit words*/
static const uint32_t keccakf_rndc[24] = {
	0x00000001, 0x00008082, 0x0000808a, 0x80008000, 0x0000808b, 0x80000001,
	0x80008081, 0x00008009, 0x0000008a, 0x00000088, 0x80008009, 0x8000000a,
	0x8000808b, 0x0000008b, 0x00008089, 0x00008003, 0x00008002, 0x00000080,
	0x0000800a, 0x8000000a, 0x80008081, 0x00008080, 0x80000001, 0x80008008
};

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

/*    primes.c  */
static int is_prime(uint32_t number)
{
    uint32_t d;

    if (number <= 1)
        return 0;

    if (number % 2 == 0 && number > 2)
        return 0;

    /* Check factors up to sqrt(number).
       To avoid computing sqrt, compare d*d <= number with 64-bit precision. */
    for (d = 3; (int64_t)d * (int64_t)d <= (int64_t)number; d += 2)
    {
        if (number % d == 0)
            return 0;
    }

    return 1;
}

static
uint32_t ethash_find_largest_prime(uint32_t upper_bound)
{
    uint32_t n = upper_bound;

    if (n < 2)
        return 0;

    if (n == 2)
        return 2;

    /* If even number, skip it. */
    if (n % 2 == 0)
        --n;

    /* Test descending odd numbers. */
    while (!is_prime(n))
        n -= 2;

    return n;
}

/* ------------------------- progpow.c ------------------------- */
typedef int bool;

#define ETHASH_DATASET_BYTES_INIT (1<<30) // 2**30
#define ETHASH_DATASET_BYTES_GROWTH (1<<23)  // 2**23
#define ETHASH_CACHE_BYTES_INIT (1<<24) // 2**24
#define ETHASH_CACHE_BYTES_GROWTH (1<<17)  // 2**17
#define ETHASH_EPOCH_LENGTH 30000U
#define ETHASH_MIX_BYTES 128
#define ETHASH_HASH_BYTES 64
#define ETHASH_DATASET_PARENTS 256
#define ETHASH_CACHE_ROUNDS 3
#define ETHASH_ACCESSES 64


typedef union
{
    uint64_t words[4];
    uint32_t hwords[8];
    uint8_t bytes[32];
} hash256;


static
uint64_t keccak_f800(const hash256* header, const uint64_t seed,
 const uint32_t *result)
{
    uint32_t st[25];

    for (int i = 0; i < 25; i++)
        st[i] = 0;
    for (int i = 0; i < 8; i++)
        st[i] = header->hwords[i];
    st[8] = (uint32_t)seed;
    st[9] = (uint32_t)(seed >> 32);
    st[10] = result[0];
    st[11] = result[1];
    st[12] = result[2];
    st[13] = result[3];

    for (int r = 0; r < 21; r++) {
        keccak_f800_round(st, r);
    }
    // last round can be simplified due to partial output
    keccak_f800_round(st, 21);

    return (uint64_t)st[0] << 32 | st[1];//should be return (uint64_t)st[0] << 32 | st[1];
}

static
void keccak_f800_arr(uint32_t* out, const hash256* header,
 const uint64_t seed, const uint32_t result[4])
{
    uint32_t st[25];

    for (int i = 0; i < 25; i++)
        st[i] = 0;
    for (int i = 0; i < 8; i++)
        st[i] = header->hwords[i];
    st[8] = (uint32_t)seed;
    st[9] = (uint32_t)(seed >> 32);
    st[10] = result[0];
    st[11] = result[1];
    st[12] = result[2];
    st[13] = result[3];

    for (int r = 0; r < 21; r++) {
        keccak_f800_round(st, r);
    }
    // last round can be simplified due to partial output
    keccak_f800_round(st, 21);

    for (int i = 0; i < 8; ++i)
        out[i] = st[i];
}


typedef union 
{
    uint64_t words[8];
    uint32_t half_words[16];
    uint8_t bytes[64];
} hash512;

typedef union 
{
        hash512 hashes[2];
        uint8_t bytes[128];        
} hash1024;

typedef union 
{
        hash512 hashes[4];
        uint64_t words[32];
        uint32_t hwords[64];
        uint8_t bytes[256];
} hash2048;

typedef struct 
{
    uint32_t epoch_number;
    uint32_t light_cache_num_items;
    hash512* light_cache;
    uint32_t full_dataset_num_items;
} epoch_context;

static
uint32_t fnv(uint32_t u, uint32_t v)
{
    return (u * 0x01000193) ^ v;
}

static hash2048* fix_endianness32(hash2048 *h) { return h;}
static uint32_t fix_endianness(uint32_t h) { return h;}

typedef struct {
	hash256 result;
	hash256 mix_hash;
} ethash_return_value_t;

#define ETHASH_NUM_DATASET_ACCESSES 64
#define full_dataset_item_parents 256

#define PROGPOW_LANES			32
#define PROGPOW_REGS			16
#define PROGPOW_CACHE_BYTES             (16*1024)
#define PROGPOW_CNT_MEM                 ETHASH_NUM_DATASET_ACCESSES
#define PROGPOW_CNT_CACHE               8
#define PROGPOW_CNT_MATH		8
#define PROGPOW_CACHE_WORDS  (PROGPOW_CACHE_BYTES / sizeof(uint32_t))


// Helper to get the next value in the per-program random sequence
#define rnd()    (kiss99(&prog_rnd))
// Helper to pick a random mix location
#define mix_src() (rnd() % PROGPOW_REGS)
// Helper to access the sequence of mix destinations
#define mix_dst() (mix_seq[(mix_seq_cnt++)%PROGPOW_REGS])

static hash512* bitwise_xor(hash512* z, const hash512* x, const hash512* y)
{
    for (size_t i = 0; i < sizeof(*z) / sizeof(z->words[0]); ++i)
        z->words[i] = x->words[i] ^ y->words[i];
    return z;
}

typedef struct {
	uint32_t z, w, jsr, jcong;
} kiss99_t;

// KISS99 is simple, fast, and passes the TestU01 suite
// https://en.wikipedia.org/wiki/KISS_(algorithm)
// http://www.cse.yorku.ca/~oz/marsaglia-rng.html
static
uint32_t kiss99(kiss99_t * st)
{
    uint32_t znew = (st->z = 36969 * (st->z & 65535) + (st->z >> 16));
    uint32_t wnew = (st->w = 18000 * (st->w & 65535) + (st->w >> 16));
    uint32_t MWC = ((znew << 16) + wnew);
    uint32_t SHR3 = (st->jsr ^= (st->jsr << 17), st->jsr ^= (st->jsr >> 13), st->jsr ^= (st->jsr << 5));
    uint32_t CONG = (st->jcong = 69069 * st->jcong + 1234567);
    return ((MWC^CONG) + SHR3);
}

static
uint32_t fnv1a(uint32_t *h, uint32_t d)
{
  return *h = (*h ^ d) * 0x1000193;
}

static
void fill_mix(
	uint64_t seed,
	uint32_t lane_id,
	uint32_t mix[PROGPOW_REGS]
)
{
    // Use FNV to expand the per-warp seed to per-lane
    // Use KISS to expand the per-lane seed to fill mix
    uint32_t fnv_hash = 0x811c9dc5;
    kiss99_t st;
	  st.z = fnv1a(&fnv_hash, (uint32_t)seed);
	  st.w = fnv1a(&fnv_hash, (uint32_t)(seed >> 32));
    st.jsr = fnv1a(&fnv_hash, lane_id);
    st.jcong = fnv1a(&fnv_hash, lane_id);
    for (int i = 0; i < PROGPOW_REGS; i++)
        mix[i] = kiss99(&st);
}

static
void swap(uint32_t *a, uint32_t *b)
{
	uint32_t t = *a;
	*a = *b;
	*b = t;
}

// Merge new data from b into the value in a
// Assuming A has high entropy only do ops that retain entropy
// even if B is low entropy
// (IE don't do A&B)
static
void merge(uint32_t *a, uint32_t b, uint32_t r)
{
	switch (r % 4)
	{
	case 0: *a = (*a * 33) + b; break;
	case 1: *a = (*a ^ b) * 33; break;
	case 2: *a = ROTL32(*a, ((r >> 16) % 32)) ^ b; break;
	case 3: *a = ROTR32(*a, ((r >> 16) % 32)) ^ b; break;
	}
}

// Random math between two input values
static
uint32_t math(uint32_t a, uint32_t b, uint32_t r)
{       
	switch (r % 11)
	{
	case 0: return a + b; break;
	case 1: return a * b; break;
	case 2: return mul_hi(a, b); break;
	case 3: return min_(a, b); break;
	case 4: return ROTL32(a, b); break;
	case 5: return ROTR32(a, b); break;
	case 6: return a & b; break;
	case 7: return a | b; break;
	case 8: return a ^ b; break;
	case 9: return clz(a) + clz(b); break;
	case 10: return popcount(a) + popcount(b); break;
	default: return 0;
	}
	return 0;
}

static 
void keccak512_64(const uint8_t data[64])
{
    keccak((uint64_t*)data, 512, data, 64);
}

void test_keccak(uint8_t* r, uint32_t bs, const uint8_t* data, uint32_t sz)
{
    keccak((uint64_t*)r, bs, data, sz);
}

static void fnv_512(hash512* u, const hash512* v)
{
    for (size_t i = 0; i < sizeof(*u) / sizeof(u->half_words[0]); ++i)
        u->half_words[i] = fnv(u->half_words[i], v->half_words[i]);
}



static
void build_light_cache(hash512* cache, uint32_t num_items, const hash256* seed)
{
    hash512 item = {0};
    keccak(item.words, 512, seed->bytes, sizeof(*seed));
    memcpy(&cache[0], &item, 64);
    for (uint32_t i = 1; i < num_items; ++i)
    {
        keccak512_64(item.bytes);
        memcpy(&cache[i], item.bytes, 64);
    }

    for (uint32_t q = 0; q < ETHASH_CACHE_ROUNDS; ++q)
    {
        for (uint32_t i = 0; i < num_items; ++i)
        {
            const uint32_t index_limit = (uint32_t)(num_items);

            // Fist index: 4 first bytes of the item as little-endian integer.
            uint32_t t = fix_endianness(cache[i].half_words[0]);
            uint32_t v = t % index_limit;

            // Second index.
            uint32_t w = (uint32_t)(num_items + (i - 1)) % index_limit;

            // Pipelining functions returning structs gives small performance boost.
            keccak512_64((uint8_t*)bitwise_xor(&cache[i], &cache[v], &cache[w]));
        }
    }
}

static
hash256* calculate_epoch_seed(hash256* epoch_seed, uint32_t epoch_number)
{
    for (uint32_t i = 0; i < epoch_number; ++i)
        keccak(epoch_seed->words, 256, epoch_seed->bytes, 32);
    return epoch_seed;
}

static void
calculate_dag_item(hash512* mix0, const epoch_context* context, uint64_t index) 
{
    const hash512* const cache = context->light_cache;

    const size_t num_half_words = sizeof(hash512) / sizeof(uint32_t);
    const uint32_t num_cache_items = context->light_cache_num_items;

    uint32_t idx32 = (uint32_t)(index);

    memcpy(mix0, &cache[index % num_cache_items], 64);

    mix0->half_words[0] ^= fix_endianness(idx32);

    // Hash and convert to little-endian 32-bit words.
    keccak512_64(mix0->bytes);

    for (uint32_t j = 0; j < full_dataset_item_parents; ++j)
    {
        uint32_t t0 = fnv(idx32 ^ j, mix0->half_words[j % num_half_words]);
        uint32_t parent_index0 = t0 % num_cache_items;
        fnv_512(mix0, &cache[parent_index0]);
    }

    // Covert 32-bit words back to bytes and hash.
    keccak512_64(mix0->bytes);
}

static hash1024* 
calculate_dataset_item(hash1024* r, const epoch_context* context, uint32_t index)
{
    hash512 n1,n2;
    uint64_t idx64 = (uint64_t)(index);
    calculate_dag_item(&n1, context, idx64*2);
    calculate_dag_item(&n2, context, idx64*2+1);
    memcpy(r->bytes, n1.bytes, 64);
    memcpy(r->bytes+64, n2.bytes, 64);
    return r;  
}

static hash2048* 
calculate_dataset_item_progpow(hash2048* r, const epoch_context* context,
 uint32_t index)
{
  hash1024 n1,n2;
  calculate_dataset_item( &n1, context, index*2);
  calculate_dataset_item( &n2, context, index*2+1);
  memcpy(r->bytes, n1.bytes, 128);
  memcpy(r->bytes+128, n2.bytes, 128);
  return r;
}

static uint32_t calculate_L1dataset_item(const epoch_context* context, uint32_t index)
{
    uint32_t idx = index/2;
    hash2048 dag;
    calculate_dataset_item_progpow(&dag, context, (idx*2+101));
    uint64_t data = dag.words[0];
    uint32_t ret;
    ret = (uint32_t)((index%2)?(data>>32):(data));
    return ret;
}

static
void progPowInit(kiss99_t* prog_rnd, uint64_t prog_seed, uint32_t mix_seq[PROGPOW_REGS])
{
    uint32_t fnv_hash = 0x811c9dc5;
    prog_rnd->z = fnv1a(&fnv_hash, (uint32_t)prog_seed);
    prog_rnd->w = fnv1a(&fnv_hash, (uint32_t)(prog_seed >> 32));
    prog_rnd->jsr = fnv1a(&fnv_hash, (uint32_t)prog_seed);
    prog_rnd->jcong = fnv1a(&fnv_hash, (uint32_t)(prog_seed >> 32));
    // Create a random sequence of mix destinations for merge()
    // guaranteeing every location is touched once
    // Uses Fisher¨CYates shuffle
    for (uint32_t i = 0; i < PROGPOW_REGS; i++)
        mix_seq[i] = i;
    for (uint32_t i = PROGPOW_REGS - 1; i > 0; i--)
    {
        uint32_t j = kiss99(prog_rnd) % (i + 1);
        swap(&(mix_seq[i]), &(mix_seq[j]));
    }
}

typedef  hash2048* (*lookup_fn2)(hash2048 *, const epoch_context*, uint32_t);
typedef  uint32_t (*lookup_fn_l1)(const epoch_context*, uint32_t);

static void progPowLoop(
	const epoch_context* context,
    const uint64_t prog_seed,
    const uint32_t loop,
    uint32_t mix[PROGPOW_LANES][PROGPOW_REGS],
    lookup_fn2  g_lut,
    lookup_fn_l1 c_lut)
{
	// All lanes share a base address for the global load
    // Global offset uses mix[0] to guarantee it depends on the load result
    uint32_t offset_g = mix[loop%PROGPOW_LANES][0] % (uint32_t)(context->full_dataset_num_items/2);
	
    hash2048 data256;
    fix_endianness32((hash2048*)g_lut(&data256, context, offset_g));
    for(int i=0;i<256;i++)data256.bytes[i] = 43;

    // Lanes can execute in parallel and will be convergent
    for (uint32_t l = 0; l < PROGPOW_LANES; l++)
    {
        // global load to sequential locations
        uint64_t data64 = data256.words[l];

        // initialize the seed and mix destination sequence
        uint32_t mix_seq[PROGPOW_REGS];
        int mix_seq_cnt = 0;
        kiss99_t prog_rnd;
        progPowInit(&prog_rnd, prog_seed, mix_seq);

        uint32_t offset, data32;
        //int max_i = max(PROGPOW_CNT_CACHE, PROGPOW_CNT_MATH);
        uint32_t max_i;
        if (PROGPOW_CNT_CACHE > PROGPOW_CNT_MATH)
            max_i = PROGPOW_CNT_CACHE;
        else
            max_i = PROGPOW_CNT_MATH;
        for (uint32_t i = 0; i < max_i; i++)
        {
            if (i < PROGPOW_CNT_CACHE)
            {
                // Cached memory access
                // lanes access random location
                offset = mix[l][mix_src()] % (uint32_t)PROGPOW_CACHE_WORDS;
                data32 = fix_endianness(c_lut(context, offset));
                data32 = 0; //del
                merge(&(mix[l][mix_dst()]), data32, rnd());
            }
            if (i < PROGPOW_CNT_MATH)
            {
                // Random Math
                data32 = math(mix[l][mix_src()], mix[l][mix_src()], rnd());
                merge(&(mix[l][mix_dst()]), data32, rnd());
            }
        }
        // Consume the global load data at the very end of the loop
        // Allows full latency hiding
        merge(&(mix[l][0]), (uint32_t)data64, rnd());
        merge(&(mix[l][mix_dst()]), (uint32_t)(data64 >> 32), rnd());
    }
}

static void
progpow_kernel(	hash256* ret, const epoch_context* context, const uint64_t seed,
 lookup_fn2 g_lut, lookup_fn_l1 c_lut )
{
    uint32_t mix[PROGPOW_LANES][PROGPOW_REGS];
    for(int i=0;i<PROGPOW_LANES;i++)for(int j=0;j<PROGPOW_REGS;j++)mix[i][j]=0;
    hash256 result;
    for (int i = 0; i < 8; i++)
        result.hwords[i] = 0;

    // initialize mix for all lanes
    for (uint32_t l = 0; l < PROGPOW_LANES; l++)
        fill_mix(seed, l, mix[l]);

    // execute the randomly generated inner loop
    for (uint32_t i = 0; i < PROGPOW_CNT_MEM; i++)
    {
        progPowLoop(context, (uint64_t)context->epoch_number, i, mix, g_lut, c_lut);
    }

    // Reduce mix data to a single per-lane result
    uint32_t lane_hash[PROGPOW_LANES];
    for (int l = 0; l < PROGPOW_LANES; l++)
    {
        lane_hash[l] = 0x811c9dc5;
        for (int i = 0; i < PROGPOW_REGS; i++)
            fnv1a(&lane_hash[l], mix[l][i]);
    }
    // Reduce all lanes to a single 128-bit result
    for (int i = 0; i < 8; i++) {
        result.hwords[i] = 0x811c9dc5;
    }
    for (int l = 0; l < PROGPOW_LANES; l++) {
        fnv1a(&result.hwords[l % 8], lane_hash[l]);
    }

	memcpy(ret, &result, sizeof(result));
}

static void
progpow(
       ethash_return_value_t* ret,
       const epoch_context *ctx,
       const hash256 *header_hash,
       uint64_t const nonce
)
{
    uint32_t result[4] = {0};
    
    uint64_t seed = keccak_f800(header_hash, nonce, result);

    progpow_kernel(&ret->mix_hash, ctx, seed, calculate_dataset_item_progpow, calculate_L1dataset_item);

    keccak_f800_arr(ret->result.hwords, header_hash, seed, ret->mix_hash.hwords);
}



static
uint32_t calculate_light_cache_num_items(uint32_t epoch_number) 
{
    uint32_t item_size = sizeof(hash512);
    uint32_t num_items_init = ETHASH_CACHE_BYTES_INIT / item_size;
    uint32_t num_items_growth = ETHASH_CACHE_BYTES_GROWTH / item_size;

    uint32_t num_items_upper_bound = num_items_init + epoch_number * num_items_growth;
    uint32_t num_items = ethash_find_largest_prime(num_items_upper_bound);
    //printf("epoch %d cache num %d\n", epoch_number, num_items);    
    return num_items;
}

static 
uint32_t calculate_full_dataset_num_items(uint32_t epoch_number) 
{
    uint32_t item_size = sizeof(hash2048);
    uint32_t num_items_init = ETHASH_DATASET_BYTES_INIT / item_size;
    uint32_t num_items_growth = ETHASH_DATASET_BYTES_GROWTH / item_size;

    uint32_t num_items_upper_bound = num_items_init + epoch_number * num_items_growth;
    uint32_t num_items = ethash_find_largest_prime(num_items_upper_bound);
    return num_items;
}


/**************API********************************/

#include <stdlib.h>
epoch_context g_ctx;
hash512* create_light_cache(uint32_t epoch) 
{
    if (g_ctx.light_cache) {
        free(g_ctx.light_cache); 
    }
    g_ctx.light_cache_num_items = calculate_light_cache_num_items(epoch);
    g_ctx.full_dataset_num_items = calculate_full_dataset_num_items(epoch);
    g_ctx.light_cache = malloc(g_ctx.light_cache_num_items * 64);

    hash256 epoch_seed = {0};
    calculate_epoch_seed(&epoch_seed, epoch);
    build_light_cache(g_ctx.light_cache, g_ctx.light_cache_num_items, &epoch_seed);
    return g_ctx.light_cache;
}

void 
get_block_progpow_hash(uint8_t header[32],
                       uint64_t nonce, uint8_t out[64])
{
    ethash_return_value_t r;
    progpow(&r, &g_ctx, (const hash256*)header, nonce);   

    memcpy(out, &r.result, 32);
    memcpy(out+32, &r.mix_hash, 32);
}

void  
get_dataset_item(uint8_t r[256], uint32_t index) {
    calculate_dataset_item_progpow((hash2048*)r, &g_ctx, index);     
}




