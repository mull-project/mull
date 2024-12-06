#!/usr/bin/bash

set -e
set -x

git submodule update --init --recursive

pip3 install -r requirements.txt

./infrastructure/generator.py cmake --os ubuntu --llvm_version $1
