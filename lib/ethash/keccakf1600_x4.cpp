/* ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
 * Copyright 2018 Pawel Bylica.
 * Licensed under the Apache License, Version 2.0. See the LICENSE file.
 */

#include "vec.hpp"
#include <cstdint>

template <typename T>
inline T rol(T x, unsigned s)
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

extern "C" void ethash_keccakf1600_x4(uint64_t state[25][4]) noexcept
{
    /* The implementation based on the "simple" implementation by Ronny Van Keer. */

    int round;

    using vec64x4 = vec<uint64_t, 4>;

    vec64x4 Aba, Abe, Abi, Abo, Abu;
    vec64x4 Aga, Age, Agi, Ago, Agu;
    vec64x4 Aka, Ake, Aki, Ako, Aku;
    vec64x4 Ama, Ame, Ami, Amo, Amu;
    vec64x4 Asa, Ase, Asi, Aso, Asu;

    vec64x4 Eba, Ebe, Ebi, Ebo, Ebu;
    vec64x4 Ega, Ege, Egi, Ego, Egu;
    vec64x4 Eka, Eke, Eki, Eko, Eku;
    vec64x4 Ema, Eme, Emi, Emo, Emu;
    vec64x4 Esa, Ese, Esi, Eso, Esu;

    vec64x4 Ba, Be, Bi, Bo, Bu;

    vec64x4 Da, De, Di, Do, Du;

    Aba = {state[0][0], state[0][1], state[0][2], state[0][3]};
    Abe = {state[1][0], state[1][1], state[1][2], state[1][3]};
    Abi = {state[2][0], state[2][1], state[2][2], state[2][3]};
    Abo = {state[3][0], state[3][1], state[3][2], state[3][3]};
    Abu = {state[4][0], state[4][1], state[4][2], state[4][3]};
    Aga = {state[5][0], state[5][1], state[5][2], state[5][3]};
    Age = {state[6][0], state[6][1], state[6][2], state[6][3]};
    Agi = {state[7][0], state[7][1], state[7][2], state[7][3]};
    Ago = {state[8][0], state[8][1], state[8][2], state[8][3]};
    Agu = {state[9][0], state[9][1], state[9][2], state[9][3]};
    Aka = {state[10][0], state[10][1], state[10][2], state[10][3]};
    Ake = {state[11][0], state[11][1], state[11][2], state[11][3]};
    Aki = {state[12][0], state[12][1], state[12][2], state[12][3]};
    Ako = {state[13][0], state[13][1], state[13][2], state[13][3]};
    Aku = {state[14][0], state[14][1], state[14][2], state[14][3]};
    Ama = {state[15][0], state[15][1], state[15][2], state[15][3]};
    Ame = {state[16][0], state[16][1], state[16][2], state[16][3]};
    Ami = {state[17][0], state[17][1], state[17][2], state[17][3]};
    Amo = {state[18][0], state[18][1], state[18][2], state[18][3]};
    Amu = {state[19][0], state[19][1], state[19][2], state[19][3]};
    Asa = {state[20][0], state[20][1], state[20][2], state[20][3]};
    Ase = {state[21][0], state[21][1], state[21][2], state[21][3]};
    Asi = {state[22][0], state[22][1], state[22][2], state[22][3]};
    Aso = {state[23][0], state[23][1], state[23][2], state[23][3]};
    Asu = {state[24][0], state[24][1], state[24][2], state[24][3]};

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

    for (std::size_t i = 0; i < 4; ++i)
    {
        state[0][i] = Aba[i];
        state[1][i] = Abe[i];
        state[2][i] = Abi[i];
        state[3][i] = Abo[i];
        state[4][i] = Abu[i];
        state[5][i] = Aga[i];
        state[6][i] = Age[i];
        state[7][i] = Agi[i];
        state[8][i] = Ago[i];
        state[9][i] = Agu[i];
        state[10][i] = Aka[i];
        state[11][i] = Ake[i];
        state[12][i] = Aki[i];
        state[13][i] = Ako[i];
        state[14][i] = Aku[i];
        state[15][i] = Ama[i];
        state[16][i] = Ame[i];
        state[17][i] = Ami[i];
        state[18][i] = Amo[i];
        state[19][i] = Amu[i];
        state[20][i] = Asa[i];
        state[21][i] = Ase[i];
        state[22][i] = Asi[i];
        state[23][i] = Aso[i];
        state[24][i] = Asu[i];
    }
}
