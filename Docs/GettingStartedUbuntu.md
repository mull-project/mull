# Installing Mull on Ubuntu 16.04 LTS (Xenial Xerus)

### Getting Docker image

install: ## Install Docker Ubuntu image
  docker pull ubuntu:xenial

  docker run -ti --name=mull_linux_container ubuntu

	docker start -ai mull_linux_container

### Installing dependencies

```bash
$ apt-get update
$ apt-get install git make vim ninja-build wget \
                  libz-dev sqlite3 libsqlite3-dev ncurses-dev \
                  cmake

# http://apt.llvm.org/
$ echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main" >> /etc/apt/sources.list.d/llvm.list
$ echo "deb-src http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main" >> /etc/apt/sources.list.d/llvm.list
$ apt-get install llvm-3.9
$ apt-get install clang-3.9

### Cloning mull

$ git clone https://github.com/mull-project/mull.git

### Running tests

make -f Makefile.ubuntu test


