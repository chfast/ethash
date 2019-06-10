# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2019 Pawel Bylica.
# Licensed under the Apache License, Version 2.0.

import unittest

import ethash


class TestEthash(unittest.TestCase):

    def test_keccak(self):
        h256 = ('c5d2460186f7233c927e7db2dcc703c0'
                'e500b653ca82273b7bfad8045d85a470')
        h512 = ('0eab42de4c3ceb9235fc91acffe746b2'
                '9c29a8c366b7c60e4e67c466f36a4304'
                'c00fa9caf9d87976ba469bcbe06713b4'
                '35f091ef2769fb160cdab33d3670680e')

        self.assertEqual(ethash.keccak_256(b'').hex(), h256)
        self.assertEqual(ethash.keccak_512(b'').hex(), h512)


if __name__ == '__main__':
    unittest.main()
