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

Mull supports different versions of LLVM. Please substitute `3.9` in
the following documentation with the actual version you use.
List of supported versions can be found in the [`LLVMCompatibility/`](https://github.com/mull-project/mull/tree/master/LLVMCompatibility)
directory in the root of the repository.

```
apt-get update
apt-get -y install wget
echo "deb http://apt.llvm.org/xenial/ llvm-toolchain-xenial-3.9 main" >> /etc/apt/sources.list.d/llvm.list
wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
```

Then install:

```bash
apt-get update
apt-get -y install git make vim ninja-build wget \
                   libz-dev sqlite3 libsqlite3-dev ncurses-dev \
                   cmake \
                   llvm-3.9 clang-3.9 llvm-3.9-dev
```

### Cloning mull

```bash
cd /opt
git clone https://github.com/mull-project/mull.git
cd mull
```

### Initialize the build system

```
make ninja.init LLVM_ROOT=/usr/lib/llvm-3.9
```

### Running tests

```bash
make test
```

### Building and installing Mull

The following command will build and install `mull-driver` into `/usr/local/bin`

```bash
make install
```

**Check if you have enough permissions to write into `/usr/local/bin` if it fails.**

Alternatively, choose a different location use the following command:

```bash
make install INSTALL_DIR=/foo/bar
```
