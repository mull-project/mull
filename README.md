# Mull

Mull is a LLVM-based tool for Mutation Testing with a strong focus on C and C++
languages.

## What is Mutation Testing

Mutation Testing, a fault-based software testing technique, serves as a way to evaluate and improve quality of software tests.

> A tool for mutation testing creates many slightly modified versions of original program and then runs a test suite against each version, which is called a _mutant_. A mutant is said to be _killed_ if the test suite detects a change to the program introduced by this mutant, or _survived_ otherwise. Each mutation of original program is created based on one of the predefined rules for program modification called _mutation operators_. Each mutant is represented by a _mutation point_: a combination of mutation operator and location of a mutation in the program’s source code. To assess the quality of a test suite mutation testing uses a metric called _mutation score_, or _mutation coverage_.

[![Slack Status](https://mutation-testing-slack.herokuapp.com/badge.svg)](https://mutation-testing.slack.com/messages/mull)

![Screenshot](/screenshot.png)

## Installation

You can install Mull using prebulit packages (TBD) or build it yourself from
sources as described here: [Hacking on Mull](/docs/documentation/hacking.md#local-development-setup)

## Usage

Here is an example using OpenSSL

### Prepare your project

To use Mull you need to build the project using Clang with the `-fembed-bitcode`
and debug information:

```
git clone https://github.com/openssl/openssl.git
cd openssl
./config -no-asm -no-shared -g -O0 -fembed-bitcode
make all
```

**Make sure you don't use clang from Xcode, it is not compatible with open-source
version of LLVM.**

### Run mull

Once the build is done you can run Mull against OpenSSL's tests:

```
mull-cxx -test-framework=CustomTest test/bio_enc_test
```

### Treat results

Mull works on level of LLVM Bitcode, therefore some of mutations may not be
represented at the source level - these are Junk Mutations. Mull can use debug
information to look back at the source code and decide whether a mutation is
junk or not.

For this to work you need to provide a compilation database,
compilation flags, or both.

It is not trivial to build the compilation database for the OpenSSL, but you
can pick required compilation flags by observing build logs.

```
mull-cxx -test-framework=CustomTest \
  -compilation-flags="-I . -I crypto/modes -I crypto/include -I include -I apps " \
  test/bio_enc_test
```

By default, clang adds implicit header search paths, they should be added to
the `-compilation-flags` explicitly. You can find them using the following
commands for C and C++ respectively:

```
clang -x c -c /dev/null -v
clang++ -x c++ -c /dev/null -v
```

Example output:

```
#include <...> search starts here:
 /usr/local/include
 /opt/llvm/5.0.0/lib/clang/5.0.0/include
 /usr/include
 /System/Library/Frameworks (framework directory)
 /Library/Frameworks (framework directory)
End of search list.
```

Which should be transformed into this form (you don't want to see mutations
inside of a system header files):

```
-isystem /usr/local/include -isystem /opt/llvm/5.0.0/lib/clang/5.0.0/include -isystem /usr/include
```

So the complete example will like this:

```
mull-cxx -test-framework=CustomTest \
  -compilation-flags="\
    -I . -I crypto/modes -I crypto/include -I include -I apps \
    -isystem /usr/local/include \
    -isystem /opt/llvm/5.0.0/lib/clang/5.0.0/include \
    -isystem /usr/include" \
  test/bio_enc_test
```

Producing the following output:

```
Extracting bitcode from executable (threads: 1): 1/1. Finished in 1215ms.
Loading bitcode files (threads: 8): 505/505. Finished in 205ms.
Compiling instrumented code (threads: 8): 505/505. Finished in 17ms.
Loading dynamic libraries (threads: 1): 1/1. Finished in 0ms.
Searching tests (threads: 1): 1/1. Finished in 2ms.
Preparing original test run (threads: 1): 1/1. Finished in 81ms.
Running original tests (threads: 1): 1/1. Finished in 690ms.
Searching mutants across functions (threads: 8): 315/315. Finished in 35ms.
Filtering out junk mutations (threads: 8): 1654/1654. Finished in 4453ms.
Preparing mutations (threads: 1): 1/1. Finished in 528ms.
Applying mutations (threads: 1): 809/809. Finished in 10ms.
Compiling original code (threads: 8): 505/505. Finished in 11ms.
Running mutants (threads: 8): 809/809. Finished in 111615ms.

Survived mutants (569/809):

/opt/mull/openssl/openssl/test/testutil/driver.c:193:29: warning: Math Add: replaced + with -
            j = rand() % (1 + i);
                            ^
/opt/mull/openssl/openssl/test/testutil/driver.c:210:17: warning: Math Add: replaced + with -
                ++num_failed;
                ^
/opt/mull/openssl/openssl/test/testutil/driver.c:212:49: warning: Math Add: replaced + with -
            test_verdict(verdict, "%d - %s", ii + 1, test_title);
.....
..... skipped
.....
/opt/mull/openssl/openssl/crypto/mem.c:249:9: warning: Remove Void Call: removed CRYPTO_free
        CRYPTO_free(str, file, line);
        ^
/opt/mull/openssl/openssl/crypto/o_str.c:86:10: warning: Math Add: replaced + with -
        l++;
         ^
/opt/mull/openssl/openssl/crypto/o_str.c:90:14: warning: Math Add: replaced + with -
    return l + strlen(src);
             ^
/opt/mull/openssl/openssl/crypto/o_str.c:84:17: warning: Negate Condition: replaced > with <=
    for (; size > 1 && *src; size--) {
                ^
Mutation score: 29%

Total execution time: 118987ms
```

## Contributing

Here is the starting point: [CONTRIBUTING.md](/CONTRIBUTING.md)

## Citation

[Mull it over: mutation testing based on LLVM (preprint)](https://lowlevelbits.org/pdfs/Mull_Mutation_2018.pdf)

```
@INPROCEEDINGS{8411727, 
author={A. Denisov and S. Pankevich}, 
booktitle={2018 IEEE International Conference on Software Testing, Verification and Validation Workshops (ICSTW)}, 
title={Mull It Over: Mutation Testing Based on LLVM}, 
year={2018}, 
volume={}, 
number={}, 
pages={25-31}, 
keywords={just-in-time;program compilers;program testing;program verification;mutations;Mull;LLVM IR;mutated programs;compiled programming languages;LLVM framework;LLVM JIT;tested program;mutation testing tool;Testing;Tools;Computer languages;Instruments;Runtime;Computer crashes;Open source software;mutation testing;llvm}, 
doi={10.1109/ICSTW.2018.00024}, 
ISSN={}, 
month={April},}
```

## Copyright

Copyright (c) 2016-2019 Alex Denisov <alex@lowlevelbits.org> and Stanislav Pankevich <s.pankevich@gmail.com>. See LICENSE for details.

