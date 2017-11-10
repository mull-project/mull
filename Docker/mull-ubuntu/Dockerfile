FROM ubuntu:xenial

MAINTAINER Stanislav Pankevich <s.pankevich@gmail.com>

RUN apt-get update && \
    apt-get -y --no-install-recommends install wget

RUN wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -

RUN echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main" >> /etc/apt/sources.list.d/llvm.list

# Dependencies:
# ca-certificates - "Problem with the SSL CA cert" when cloning Mull otherwise.
# libz, ncurses - LLVM's dependencies.
# sqlite3 - Mull generates SQLite reports.
RUN apt-get update && \
    apt-get -y --no-install-recommends install \
        ca-certificates \
        git \
        vim \
        libz-dev sqlite3 libsqlite3-dev ncurses-dev \
        make cmake ninja-build \
        llvm-3.9 clang-3.9 llvm-3.9-dev

RUN cd /opt && \
    git clone https://github.com/mull-project/mull.git


