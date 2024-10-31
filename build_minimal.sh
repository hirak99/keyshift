#!/bin/bash

# Builds only the main `keyshift` binary.

set -uexo pipefail
readonly MY_PATH=$(realpath $(dirname "$0"))
mkdir -p "$MY_PATH/build"
cd "$MY_PATH/build"

cmake ../src -DENABLE_TESTS=OFF
make -j keyshift
