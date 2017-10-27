# How to run Mull

<!-- START doctoc generated TOC please keep comment here to allow auto update -->
<!-- DON'T EDIT THIS SECTION, INSTEAD RE-RUN doctoc TO UPDATE -->
**Table of Contents**  *generated with [DocToc](https://github.com/thlorenz/doctoc)*

- [Step 1: Building Mull](#step-1-building-mull)
- [Step 2: Getting LLVM bitcode](#step-2-getting-llvm-bitcode)
  - [Example: Fmt library](#example-fmt-library)
- [Step 3: Creating config.yml file](#step-3-creating-configyml-file)
- [Step 4: Running Mull](#step-4-running-mull)
- [Step 5: Generating HTML report](#step-5-generating-html-report)
- [Known issues](#known-issues)
  - [LLVM ERROR: Program used external function which could not be resolved!](#llvm-error-program-used-external-function-which-could-not-be-resolved)
  - [error: Invalid record](#error-invalid-record)

<!-- END doctoc generated TOC please keep comment here to allow auto update -->

## Step 1: Building Mull

You need to have a `mull-driver` binary on your machine. We don't distribute
Mull as binary yet so you have to build it yourself.

See:

- [Getting started on Mac OS](/Docs/GettingStartedMacOS.md)
- [Getting started on Ubuntu](/Docs/GettingStartedUbuntu.md)
- [Getting started on CentOS](/Docs/GettingStartedCentOS7.md)

## Step 2: Getting LLVM bitcode

There are at least 3 different options to obtain LLVM bitcode. Keep in mind
that in each case you also want to add the following flags:

`-g` Tells `clang` to generate a debug information which helps Mull to generate
useful information about the code that is tested, for example the file/line
information that is attached to mutation points.

`-O0` This flag is enabled by default if not specified. However some projects
might have exotic build systems where `-O0` must be explicitly enforced, for
example: `s/-O3/-O0/g`, so it is a good practice to always specify this flag to make sure Mull works on unoptimized code.

### -emit-llvm

```bash
$ clang -g -O0 -emit-llvm -c file.cpp -o file.bc
$ file file.bc
LLVM bitcode, wrapper x86_64
```

### LTO

The `-flto` flag also generates LLVM bitcode but it is easier to integrate to existing build systems than `-emit-llvm`, because you can just add `-flto`
to the existing build rules of your object files.

```bash
$ clang -g -O0 -flto -c file.cpp -o file.o
$ file file.o
LLVM bitcode, wrapper x86_64
```

Notice that `-flto` produces LLVM bitcode files, not object files in this case!

### LTO or emit-llvm integrated to a build system

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

## Step 3: Creating config.yml file

See [example of config.yml](/Docs/config.example.yml).

TODO

## Step 4: Running Mull

TODO

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

TODO
