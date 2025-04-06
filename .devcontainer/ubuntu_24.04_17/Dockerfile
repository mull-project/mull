FROM ubuntu:24.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-14-dev \
  libclang-14-dev \
  libc++-14-dev \
  libc++abi-14-dev \
  clang-14

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-15-dev \
  libclang-15-dev \
  libc++-15-dev \
  libc++abi-15-dev \
  clang-15

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-16-dev \
  libclang-16-dev \
  libc++-16-dev \
  libc++abi-16-dev \
  clang-16

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-17-dev \
  libclang-17-dev \
  libc++-17-dev \
  libc++abi-17-dev \
  clang-17

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-18-dev \
  libclang-18-dev \
  libc++-18-dev \
  libc++abi-18-dev \
  clang-18

USER ubuntu
