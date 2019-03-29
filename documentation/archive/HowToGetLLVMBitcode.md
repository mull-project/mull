# How to get LLVM Bitcode

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [-emit-llvm](#-emit-llvm)
- [LTO](#lto)
- [LTO or emit-llvm integrated to a build system](#lto-or-emit-llvm-integrated-to-a-build-system)
  - [Example: Fmt library](#example-fmt-library)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

There are at least 3 different options to obtain LLVM bitcode. Keep in mind
that in each case you also want to add the following flags:

`-g` Tells `clang` to generate a debug information which helps Mull to generate
useful information about the code that is tested, for example the file/line
information that is attached to mutation points.

`-O0` This flag is enabled by default if not specified. However some projects
might have exotic build systems where `-O0` must be explicitly enforced, for
example: `s/-O3/-O0/g`, so it is a good practice to always specify this flag to make sure Mull works on unoptimized code.

**Important note:** Do not use Mac OS's Clang to compile the
files to a bitcode. Mull is built on top of Clang/LLVM 3.9 which does not work
with Clang/LLVM of Apple. If you followed the Getting Started instructions
you should have LLVM 3.9 in place, we recommend to use `llvm-3.9/bin/clang`
to build LLVM bitcode.

## -emit-llvm

```bash
$ clang -g -O0 -emit-llvm -c file.cpp -o file.bc
$ file file.bc
LLVM bitcode, wrapper x86_64
```

## LTO

The `-flto` flag also generates LLVM bitcode but it is easier to integrate to existing build systems than `-emit-llvm`, because you can just add `-flto`
to the existing build rules of your object files.

```bash
$ clang -g -O0 -flto -c file.cpp -o file.o
$ file file.o
LLVM bitcode, wrapper x86_64
```

Notice that `-flto` produces LLVM bitcode files, not object files in this case!

## LTO or emit-llvm integrated to a build system

Depending on how you build your project it might be easier or harder to get LLVM
bitcode out of your project.

The easiest case is when the project is based on CMake. Another easy case
is a Make-based system where conventions with CC, CXX, CXXFLAGS are followed.
In these cases it is just enough to add `-flto` flag (see Fmt library example
 below).

If your case is special then you might need to write a script that adds
`-emit-llvm` or `-flto` to your compile rules.


### Example: Fmt library

> fmt is an open-source formatting library for C++. It can be used as a safe alternative to printf or as a fast alternative to IOStreams.

```
git clone https://github.com/fmtlib/fmt.git
mkdir fmt-build
cd fmt-build
export CC=/opt/llvm-3.9.0/bin/clang
export CXX=/opt/llvm-3.9.0/bin/clang++
export CFLAGS="-flto -g -O0"
export CXXFLAGS="-flto -g -O0"
export CPPFLAGS="-flto -g -O0"
cmake -G "Unix Makefiles" ../fmt
find `pwd` -iname "*.o" > ../fmt.bitcode.list
```
