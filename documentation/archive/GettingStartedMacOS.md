# Getting started on Mac OS

## Getting LLVM

Get LLVM from [LLVM Download Page](http://releases.llvm.org/).

Mull supports different versions of LLVM. Please substitute `3.9` in
the following documentation with the actual version you use.
List of supported versions can be found in the [`LLVMCompatibility/`](https://github.com/mull-project/mull/tree/master/LLVMCompatibility)
directory in the root of the repository.

```bash
cd /opt
wget http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz
tar xvf clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz
mv clang+llvm-3.9.0-x86_64-apple-darwin llvm-3.9
```

## Getting dependencies

```bash
# These are needed by LLVM
brew install zlib ncurses

# SQLite is needed for Mull to generate reports.
brew install sqlite

# These are development dependencies
brew install cmake ninja
```

## Getting Mull

```bash
git clone https://github.com/mull-project/mull.git
cd mull
```

## Initialize the build system

```
make ninja.init LLVM_ROOT=/opt/llvm-3.9
```

## Running tests

```bash
make test
```

## Building Mull as Xcode project

```bash
make xcode.init
open BuildXcode/Mull.xcodeproj
```

## Building and installing Mull

The following command will build and install `mull-driver` into `/usr/local/bin`

```bash
make install
```

**Check if you have enough permissions to write into `/usr/local/bin` if it fails.**

Alternatively, choose a different location use the following command:

```bash
make install INSTALL_DIR=/foo/bar
```

