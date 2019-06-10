# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2019 Pawel Bylica.
# Licensed under the Apache License, Version 2.0.

from _ethash import ffi, lib


def keccak_256(data):
    hash = lib.ethash_keccak256(ffi.from_buffer(data), len(data))
    return ffi.unpack(hash.str, len(hash.str))


def keccak_512(data):
    hash = lib.ethash_keccak512(ffi.from_buffer(data), len(data))
    return ffi.unpack(hash.str, len(hash.str))
