#!/bin/bash

set -uexo pipefail
readonly MY_PATH=$(realpath $(dirname "$0"))
cd "$MY_PATH/build"

cmake ../src
make -j
ctest
