FROM ubuntu:22.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-13-dev \
  libclang-13-dev \
  libc++-13-dev \
  libc++abi-13-dev \
  clang-13
