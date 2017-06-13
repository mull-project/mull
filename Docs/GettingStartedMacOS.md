# Getting started on Mac OS

## Getting LLVM

Get LLVM 3.9 from [LLVM Download Page](http://releases.llvm.org/).

```bash
cd /opt
wget http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz
tar xvf
mv clang+llvm-3.9.0-x86_64-apple-darwin llvm-3.9
```

## Getting dependencies

```bash
# These are needed by LLVM
brew install zlib
brew install ncurses

# SQLite is needed for Mull to generate reports.
brew install sqlite
```

## Getting Mull

```bash
git clone https://github.com/mull-project/mull.git
cd mull
```

## Running tests

```bash
make -f Makefile.macos test
```

## Building Mull as Xcode project

```bash
make -f Makefile.macos build_xcode
open Mull.xcodeproj
```
