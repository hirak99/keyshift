#!/bin/bash

# Builds all binaries including tests, and then runs tests.

set -uexo pipefail
readonly MY_PATH=$(realpath $(dirname "$0"))
mkdir -p "$MY_PATH/build"
cd "$MY_PATH/build"

cmake ../src -DENABLE_TESTS=ON
make -j
ctest
