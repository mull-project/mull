# Getting started on Ubuntu 16.04 LTS (Xenial Xerus)

### Getting Docker image

```bash
$ docker pull ubuntu:xenial

# This command creates and starts a new container.
$ docker run -ti --name=mull_linux_container ubuntu:xenial
# This command starts existing container, use it later.
$ docker start -ai mull_linux_container
```

### Installing dependencies

Add LLVM sources for binary distributions as recommended by
[http://apt.llvm.org/](http://apt.llvm.org/):

```
$ echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main" >> /etc/apt/sources.list.d/llvm.list
$ apt-get update
$ apt-get -y install wget
$ wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
```

Then install:

```bash
$ apt-get -y install git make vim ninja-build wget \
                     libz-dev sqlite3 libsqlite3-dev ncurses-dev \
                     cmake \
                     llvm-3.9 clang-3.9
```

### Cloning mull

```bash
$ cd /opt
$ git clone https://github.com/mull-project/mull.git
$ cd mull
```

### Running tests

```bash
make -f Makefile.ubuntu test
```

