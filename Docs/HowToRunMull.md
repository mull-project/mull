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

You need to have a `mull-driver` binary on your machine. We don't distribute
Mull as binary yet so you have to build it yourself.

See:

- [Getting started on Mac OS](/Docs/GettingStartedMacOS.md)
- [Getting started on Ubuntu](/Docs/GettingStartedUbuntu.md)
- [Getting started on CentOS](/Docs/GettingStartedCentOS7.md)

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

See [example of config.yml](/Docs/config.example.yml).

TODO

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
