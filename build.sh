#!/bin/bash

set -uexo pipefail
readonly MY_PATH=$(realpath $(dirname "$0"))
mkdir -p "$MY_PATH/build"
cd "$MY_PATH/build"

cmake ../src
make -j
ctest
