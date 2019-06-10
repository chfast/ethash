#!/usr/bin/env bash

# ethash: C/C++ implementation of Ethash, the Ethereum Proof of Work algorithm.
# Copyright 2019 Pawel Bylica.
# Licensed under the Apache License, Version 2.0.

set -e

if [ -n "$APPVEYOR" ]; then
    PYTHON_PATHS="/c/Python37-x64:/c/Python37-x64/Scripts /c/Python36-x64:/c/Python36-x64/Scripts /c/Python35-x64:/c/Python35-x64/Scripts"
fi

PATH_ORIG=$PATH
for p in $PYTHON_PATHS
do
    export PATH="$p:$PATH_ORIG"
    echo '***'
    python --version
    echo '***'
    pip --version
    pip install wheel
    python setup.py build_ext --skip-cmake-build
    python setup.py bdist_wheel --skip-build
done
