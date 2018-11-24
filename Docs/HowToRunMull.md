# How to run Mull

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Step 1: Building Mull](#step-1-building-mull)
- [Step 2: Getting LLVM bitcode](#step-2-getting-llvm-bitcode)
- [Step 3: Creating config.yml file](#step-3-creating-configyml-file)
- [Step 4: Running Mull](#step-4-running-mull)
- [Step 5: Generating HTML report](#step-5-generating-html-report)
- [Example: Hello World](#example-hello-world)
- [Known issues](#known-issues)
  - [LLVM ERROR: Program used external function which could not be resolved!](#llvm-error-program-used-external-function-which-could-not-be-resolved)
  - [error: Invalid record](#error-invalid-record)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Step 1: Building Mull

We don't distribute Mull as binary yet so you have to build it yourself.

### Dependencies: sqlite3 & cmake

Mull works on Linux, macOS and FreeBSD. It directly depends on `sqlite3`,
`clang` and `LLVM`. Also, you need `cmake` to build it.

You can install `sqlite3` and `cmake` using your default package manager.
Some examples:

#### Ubuntu/Debian

```
apt-get install -y sqlite3 libsqlite3-dev cmake
```

#### CentOS

```
yum install -y libsq3-devel
```

CentOS uses very old CMake version, install one based on [How to install latest cmake version in Linux/Ubuntu from command line?](https://askubuntu.com/a/595441/306213):

```bash
cd /opt
mkdir cmake
wget https://cmake.org/files/v3.8/cmake-3.8.2-Linux-x86_64.sh
sh cmake-3.8.2-Linux-x86_64.sh --prefix=/opt/cmake # Yes to license, No to include
ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
```

#### FreeBSD

```
cd /usr/ports/databases/sqlite3/ && make install clean
cd /usr/ports/devel/cmake/ && make install clean
# or
pkg install sqlite3 cmake
```

#### macOS

```
brew install sqlite3 cmake
```

### Dependencies: LLVM & clang

As of LLVM and Clang: Mull can be built against the following versions:

  - 3.9
  - 4.0
  - 5.0
  - 6.0

It is recommended to use precompiled binaries from LLVM website: http://releases.llvm.org.

An example installation that will be used across this howto follows. Please,
pick the appropriate version for your OS.

```
cd /opt
wget http://releases.llvm.org/3.9.0/clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz
tar xvf clang+llvm-3.9.0-x86_64-apple-darwin.tar.xz
mv clang+llvm-3.9.0-x86_64-apple-darwin llvm-3.9
```

For Ubuntu/Debian it is recommended to use official LLVM repositories: https://apt.llvm.org

### Build Mull

In order to build Mull you need to provide a full path to the LLVM
installation.

```
git clone https://github.com/mull-project/mull.git
mkdir build
cd build
cmake -DPRECOMPILED_LLVM_DIR=/opt/llvm-3.9 ../mull
make mull-driver mull-reporter
sudo make install
```

By default, it will be installed into `/usr/local/bin`.

### Known build issues

If you see errors such as:

```
Toolchain.cpp:(.text._ZN4mull9ToolchainC2ERNS_6ConfigE+0x16c): undefined reference to `llvm::EngineBuilder::selectTarget(llvm::Triple const&, llvm::StringRef, llvm::StringRef, llvm::SmallVectorImpl<std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > > const&)'
```

then you need to clean the build folder and re-run CMake with an additional option:

```
cmake -DPRECOMPILED_LLVM_DIR=/opt/llvm-3.9 -DCMAKE_CXX_FLAGS=-D_GLIBCXX_USE_CXX11_ABI=0 ../mull
```

## Step 2: Getting LLVM bitcode

To run your project with Mull you need to build your project as LLVM bitcode.

Read the dedicated tutorial before going further:
[How to get LLVM Bitcode](/Docs/HowToGetLLVMBitcode.md).

Mull works with a project that has tests so both project files and its test
files have to be compiled to LLVM bitcode.

Additionally, if the project uses a test framework like Google Test, this
framework has to be compiled into LLVM IR.

See [Hello World example](#example-hello-world) below.

## Step 3: Creating config.yml file

See examples or pick up the features you need from [this document](/Docs/Config.md).

## Step 4: Running Mull

Once `config.yml` file is in place, run `mull-driver` with a path to this file
as the first argument:

```bash
mull-driver /opt/mull/Examples/HelloWorld/build/config.yml
```

When `mull-driver` finishes running your project it produces something like
this:

```
Results can be found at '/opt/mull/Examples/HelloWorld/1508698142.sqlite'
```

## Step 5: Generating HTML report

The reporting is done by a Ruby gem that lives in a separate repository.

```bash
git clone https://github.com/mull-project/mull-reporter-sqlite
cd mull-reporter-sqlite
bundle # this installs the dependencies from the Ruby world
```

To run report you take the path to `.sqlite` file generated at step 4 and run:

```bash
REPORT=/path/to/file.sqlite make build
```

Note: You might have to tinker with Ruby stuff a bit. Please ask if you have
any troubles.

## Example: Hello World

The test project [Examples/HelloWorld](https://github.com/mull-project/mull/tree/master/Examples/HelloWorld) demonstrates how to run Mull on a simple CPP file
with tests based on Google Test. It exercises the following steps:

- Hello.cpp file is compiled to LLVM bitcode: `Hello.bc`.
- Google Test framework's sources, which Mull has in the project tree, are
 compiled to LLVM bitcode: `gtest.bc`.
- file `bitcodelist` is created with the following contents:

```yaml
/opt/mull/Examples/HelloWorld/build/HelloWorld.bc
/opt/mull/Examples/HelloWorld/build/gtest.bc
```

- file `config.yml` created with the following contents:

```yaml
bitcode_file_list: /opt/mull/Examples/HelloWorld/build/bitcodelist
```

- `mull-driver /opt/mull/Examples/HelloWorld/build/config.yml` is called.

## Known issues

### LLVM ERROR: Program used external function which could not be resolved!

```
LLVM ERROR: Program used external function '__ZN7testing4TestD2Ev' which could not be resolved!
```

TODO

### error: Invalid record

```
error: Invalid record (Producer: 'APPLE_1_900.0.38_0' Reader: 'LLVM 3.9.0')
```

This error indicates that you compiled your files to LLVM bitcode with default
Mac OS's `clang` compiler which does not work well with LLVM 3.9 that Mull is
based on. Check [How to get LLVM Bitcode](/Docs/HowToGetLLVMBitcode.md) for further
details.
