#!/usr/bin/bash

set -e
set -x

git config --global --add safe.directory /workspaces/mull

git submodule update --init --recursive

pip3 install -r requirements.txt

./infrastructure/generator.py cmake --os ubuntu --llvm_version $1

./end2end-tests/setup_end2end_tests.sh
