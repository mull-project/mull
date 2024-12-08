FROM ubuntu:22.04

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y \
  llvm-14-dev \
  libclang-14-dev \
  libc++-14-dev \
  libc++abi-14-dev \
  clang-14
