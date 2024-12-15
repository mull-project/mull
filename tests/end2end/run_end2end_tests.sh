#!/bin/bash

set -e
set -x

OS_NAME=$1
LLVM_VERSION=$2
BUILD_DIR=build.$OS_NAME.$LLVM_VERSION.dir

if (( $LLVM_VERSION < 16 )); then
  EXTRA_FLAGS=-fexperimental-new-pass-manager
fi

if [ "$OS_NAME" == "macos" ]
then
  sudo cmake --install $BUILD_DIR
  PLUGIN_PATH=/usr/local/lib/mull-ir-frontend-$LLVM_VERSION
  MULL_CC=/opt/homebrew/opt/llvm@$LLVM_VERSION/bin/clang
  MULL_CXX=/opt/homebrew/opt/llvm@$LLVM_VERSION/bin/clang++
else
  dpkg -i $BUILD_DIR/*.deb
  PLUGIN_PATH=/usr/lib/mull-ir-frontend-$LLVM_VERSION
  MULL_CC=clang-$LLVM_VERSION
  MULL_CXX=clang++-$LLVM_VERSION
fi

which mull-runner-$LLVM_VERSION

pushd `dirname "$0"`

export MULL_CONFIG=$PWD/mull.yml

function test_fmt() {
  pushd fmt
  rm -rf fmt.build.dir
  mkdir fmt.build.dir
  pushd fmt.build.dir

  cmake -G Ninja \
    -DCMAKE_CXX_FLAGS="-grecord-command-line -fpass-plugin=$PLUGIN_PATH $EXTRA_FLAGS" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=$MULL_CC \
    -DCMAKE_CXX_COMPILER=$MULL_CXX \
    ..

  ninja
  for test_case in $(ctest --show-only | grep 'Test .*:' | awk ' { print $3} ');
  do
    if [ "$test_case" == "posix-mock-test" ]
    then
      echo "Skipping $test_case"
      continue
    fi
    echo "Testing $test_case"
    mull-runner-$LLVM_VERSION --allow-surviving --reporters SQLite --report-name fmt ./bin/$test_case
  done

  mull-reporter-$LLVM_VERSION --allow-surviving ./fmt.sqlite

  popd # fmt.build.dir
  popd # fmt
}


function test_openssl() {
  pushd openssl

  env CC=$MULL_CC ./config -g -O0 \
     $EXTRA_FLAGS \
    -grecord-command-line \
    -fpass-plugin=$PLUGIN_PATH

  make build_generated -j
  make ./test/bio_enc_test -j

  if [ "$OS_NAME" == "macos" ]
  then
    install_name_tool -change /usr/local/lib/libcrypto.3.dylib $PWD/libcrypto.3.dylib ./test/bio_enc_test
  fi

  env LD_LIBRARY_PATH=. ./test/bio_enc_test
  env LD_LIBRARY_PATH=. mull-runner-$LLVM_VERSION --allow-surviving ./test/bio_enc_test

  popd # openssl
}

test_fmt
test_openssl
