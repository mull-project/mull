# Getting started on CentOS 7

**This page could be outdated!**

## Installing dependencies

```
yum install epel-release
yum updateinfo
yum install -y make git wget ninja-build zlib-devel ncurses-devel libsq3-devel
```

## Installing latest CMake

The following is based on [How to install latest cmake version in Linux/Ubuntu from command line?](https://askubuntu.com/a/595441/306213):

```bash
cd /opt
mkdir cmake
wget https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.sh
sh cmake-3.8.2-Linux-x86_64.sh --prefix=/opt/cmake # Yes to license, No to include
ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
```

## Installing Clang and LLVM 3.9

The following is based on [How to install Clang and LLVM 3.9 on CentOS 7](https://stackoverflow.com/questions/44219158/how-to-install-clang-and-llvm-3-9-on-centos-7/44349624#44349624):

```bash
mkdir /opt/llvm-sources
cd /opt/llvm-sources
# Compiler RT
wget https://copr-be.cloud.fedoraproject.org/results/alonid/llvm-3.9.0/epel-7-x86_64/00503631-compiler-rt-3.9.0/compiler-rt-3.9.0-3.9.0-3.el7.centos.alonid.x86_64.rpm
# Clang libs
wget https://copr-be.cloud.fedoraproject.org/results/alonid/llvm-3.9.0/epel-7-x86_64/00505197-clang-3.9.0/clang-3.9.0-libs-3.9.0-3.1.el7.centos.alonid.x86_64.rpm
# Clang
wget https://copr-be.cloud.fedoraproject.org/results/alonid/llvm-3.9.0/epel-7-x86_64/00505197-clang-3.9.0/clang-3.9.0-3.9.0-3.1.el7.centos.alonid.x86_64.rpm
# LLVM libs
wget https://copr-be.cloud.fedoraproject.org/results/alonid/llvm-3.9.0/epel-7-x86_64/00503606-llvm-3.9.0/llvm-3.9.0-libs-3.9.0-7.el7.centos.alonid.x86_64.rpm
# LLVM
wget https://copr-be.cloud.fedoraproject.org/results/alonid/llvm-3.9.0/epel-7-x86_64/00503606-llvm-3.9.0/llvm-3.9.0-3.9.0-7.el7.centos.alonid.x86_64.rpm
# LLVM devel
wget https://copr-be.cloud.fedoraproject.org/results/alonid/llvm-3.9.0/epel-7-x86_64/00503606-llvm-3.9.0/llvm-3.9.0-devel-3.9.0-7.el7.centos.alonid.x86_64.rpm

yum install -y compiler-rt-3.9.0-3.9.0-3.el7.centos.alonid.x86_64.rpm
yum install -y clang-3.9.0-libs-3.9.0-3.1.el7.centos.alonid.x86_64.rpm
yum install -y clang-3.9.0-3.9.0-3.1.el7.centos.alonid.x86_64.rpm
yum install -y llvm-3.9.0-libs-3.9.0-7.el7.centos.alonid.x86_64.rpm
yum install -y llvm-3.9.0-3.9.0-7.el7.centos.alonid.x86_64.rpm
yum install -y llvm-3.9.0-devel-3.9.0-7.el7.centos.alonid.x86_64.rpm
```

Maybe we can adjust CMake files later, but for now symlinks to the job.

```bash
ln -s /opt/llvm-3.9.0/lib64/libLLVM-3.9.so /opt/llvm-3.9.0/lib64/libLLVM.so
ln -s /opt/llvm-3.9.0/lib64 /opt/llvm-3.9.0/lib
ln -s /usr/bin/ninja-build /usr/bin/ninja
```

## Getting Mull

```bash
cd /opt
git clone https://github.com/mull-project/mull.git
cd mull
```

### Running Mull's tests

```bash
# /opt/mull
make -f Makefile.centos test
```

### Installing Mull

`install` make task installs to default directory: `/opt/mull-driver`.

```bash
# /opt/mull
make -f Makefile.centos install
```
