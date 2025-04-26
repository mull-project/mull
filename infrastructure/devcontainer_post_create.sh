#!/usr/bin/bash

set -e
set -x

mkdir -p .vscode

pip3 install -r requirements.txt
pre-commit install

# ./infrastructure/generator.py cmake --os $1 --llvm_version $2
# ./infrastructure/generator.py vscode --os $1 --llvm_version $2
