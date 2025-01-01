FROM ubuntu:24.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-18-dev \
  libclang-18-dev \
  libc++-18-dev \
  libc++abi-18-dev \
  clang-18
