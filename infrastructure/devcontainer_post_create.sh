#!/usr/bin/bash

set -e
set -x

git submodule update --init --recursive

./infrastructure/generator.py cmake --os ubuntu --llvm_version $1
