FROM ubuntu:20.04

RUN apt-get update
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y vim wget ninja-build \
  libz-dev sqlite3 libsqlite3-dev \
  ncurses-dev libstdc++6 \
  binutils liblzma-dev \
  openssh-client
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y llvm-12-dev libclang-12-dev libc++-12-dev \
  libc++abi-12-dev clang-12
