#!/usr/bin/bash

set -e
set -x

LLVM_VERSION=$1

# Install mull system-wide
dpkg -i build.dir/*.deb

pushd `dirname "$0"`

export MULL_CONFIG=$PWD/mull.yml

function test_fmt() {
  pushd fmt
  rm -rf fmt.build.dir
  mkdir fmt.build.dir
  pushd fmt.build.dir

  cmake -G Ninja \
    -DCMAKE_CXX_FLAGS="-fexperimental-new-pass-manager -grecord-command-line -fpass-plugin=/usr/lib/mull-ir-frontend-$LLVM_VERSION" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang-$LLVM_VERSION \
    -DCMAKE_CXX_COMPILER=clang++-$LLVM_VERSION \
    ..

  ninja core-test
  ./bin/core-test

  mull-runner-$LLVM_VERSION --allow-surviving ./bin/core-test

  popd # fmt.build.dir
  popd # fmt
}


function test_openssl() {
  pushd openssl

  env CC=clang-$LLVM_VERSION ./config -g -O0 \
    -grecord-command-line \
    -fexperimental-new-pass-manager \
    -fpass-plugin=/usr/lib/mull-ir-frontend-$LLVM_VERSION

  make build_generated -j
  make ./test/bio_enc_test -j
  env LD_LIBRARY_PATH=. ./test/bio_enc_test
  env LD_LIBRARY_PATH=. mull-runner-$LLVM_VERSION --allow-surviving ./test/bio_enc_test

  popd # openssl
}

test_fmt
test_openssl
