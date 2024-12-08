FROM ubuntu:24.04

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-15-dev \
  libclang-15-dev \
  libc++-15-dev \
  libc++abi-15-dev \
  clang-15
