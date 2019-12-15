#!/usr/bin/env bash

mkdir -p packages

function prepare_package () {
printf "Preparing package for $1 (LLVM $2)..."
  export LLVM_VERSION=$2
  vagrant up $1 --provision 2> ./packages/$1.err.log > ./packages/$1.out.log
  vagrant destroy -f $1 2>> ./packages/$1.err.log >> ./packages/$1.out.log
  printf "Done.\n"
}

prepare_package debian 6.0.0
#prepare_package freebsd 8.0.0
prepare_package ubuntu16 8.0.0
prepare_package ubuntu18 8.0.0
prepare_package ubuntu18 9.0.0
prepare_package macos 8.0.0
prepare_package macos 9.0.0