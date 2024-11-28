#!/usr/bin/bash

set -e
set -x

git submodule update --init --recursive

mkdir -p build.debug.dir
cd build.debug.dir
cmake -G Ninja -DCMAKE_PREFIX_PATH="/usr/lib/llvm-$1/cmake/;/usr/lib/cmake/clang-$1/" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_C_COMPILER=clang-$1 \
  -DCMAKE_CXX_COMPILER=clang++-$1 \
  -DCMAKE_CXX_FLAGS=-D_GLIBCXX_USE_CXX11_ABI=1 \
  ..
