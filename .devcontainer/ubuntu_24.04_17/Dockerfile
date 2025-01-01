FROM ubuntu:24.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-17-dev \
  libclang-17-dev \
  libc++-17-dev \
  libc++abi-17-dev \
  clang-17
