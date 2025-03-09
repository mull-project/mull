#!/bin/bash

set -e
set -x

cd `dirname "$0"`

rm -rf fmt openssl

mkdir fmt
pushd fmt
git init
git fetch --depth 1 https://github.com/fmtlib/fmt.git 123913715afeb8a437e6388b4473fcc4753e1c9a
git checkout 123913715afeb8a437e6388b4473fcc4753e1c9a
popd

mkdir openssl
pushd openssl
git init
git fetch --depth 1 https://github.com/openssl/openssl.git a4e01187d3648d9ce99507097400902cf21f9b55
git checkout a4e01187d3648d9ce99507097400902cf21f9b55
popd
