# Mull

Mull is a LLVM-based tool for Mutation Testing with a strong focus on C and C++ languages.

## What is Mutation Testing

Mutation Testing, a fault-based software testing technique, serves as a way to evaluate and improve quality of software tests.

> A tool for mutation testing creates many slightly modified versions of original program and then runs a test suite against each version, which is called a _mutant_. A mutant is said to be _killed_ if the test suite detects a change to the program introduced by this mutant, or _survived_ otherwise. Each mutation of original program is created based on one of the predefined rules for program modification called _mutation operators_. Each mutant is represented by a _mutation point_: a combination of mutation operator and location of a mutation in the program’s source code. To assess the quality of a test suite mutation testing uses a metric called _mutation score_, or _mutation coverage_.

[![Slack Status](https://mutation-testing-slack.herokuapp.com/badge.svg)](https://mutation-testing.slack.com/messages/mull)

![Screenshot](/screenshot.png)

## Installation

You can install Mull using [prebuilt packages](https://github.com/mull-project/mull/releases) or build it yourself from
sources as described here: [Hacking on Mull](https://mull.readthedocs.io/en/latest/HackingOnMull.html)

**Please, note:** If you're installing on MacOS you will need to use a different Clang than the one bundled with Xcode.

## Usage

Please, read the intro first, then look at examples.

### Intro

Mull works on the level of LLVM Bitcode relying on debug information to show
results, therefore you should build your project with `-fembed-bitcode` and
debug information enabled.

##### Junk Mutations

Not every mutation found at Bitcode level can be represented at the source
level, so depending on your project Mull may show lots of Junk Mutations. Mull
can filter them out by looking at the source code, but for that you need to
provide [compilation database](https://clang.llvm.org/docs/JSONCompilationDatabase.html),
or compilation flags, or both.

**Please, note:** Clang adds implicit header search paths, which must be added
explicitly via `-compilation-flags`. You can get them using the following
commands, for C and C++ respectively:

```
> clang -x c -c /dev/null -v
... skipped
#include <...> search starts here:
 /usr/local/include
 /opt/llvm/5.0.0/lib/clang/5.0.0/include
 /usr/include
 /System/Library/Frameworks (framework directory)
 /Library/Frameworks (framework directory)
End of search list.
```

```
> clang++ -x c++ -c /dev/null -v
#include <...> search starts here:
 /opt/llvm/5.0.0/include/c++/v1
 /usr/local/include
 /opt/llvm/5.0.0/lib/clang/5.0.0/include
 /usr/include
 /System/Library/Frameworks (framework directory)
 /Library/Frameworks (framework directory)
End of search list.
```

##### Dynamic Libraries

If your project depends on dynamic libraries other than `libc` or `lib[std]c++`,
then you may need to provide library search paths manually: Mull does not assume
any library search paths because it is very platform dependent.

On Linux, the library search paths are controlled via this file `/etc/ld.so.conf`,
on macOS it should just work.

##### Test Frameworks

Currently, Mull supports [Google Test](https://github.com/google/googletest)
framework and so called Custom Test framework. For Google Test, Mull assesses
each test individually, giving a better overview. For Custom Test, Mull treats
the whole program as one test.

##### Mutators

Mull supports several mutators, you can specify which ones to use. See the full
list here:

```
mull-cxx --help
```

### Configuration

#### Generating HTML report (Mutation Testing Elements)

Mull supports 
[Mutation Testing Elements](https://github.com/stryker-mutator/mutation-testing-elements) 
which is an HTML reporter for a number of mutation testing tools. 

Mull only generates a JSON file with a mutation testing report. The JSON file
can then be read by the Mutation Testing Elements's HTML/Javascript code and 
rendered in a browser. See the mutation-testing-elements's 
[simple-example](https://github.com/stryker-mutator/mutation-testing-elements/tree/059732743f75ea1abfcc2f093cef7a8db8798739/packages/mutation-testing-elements/testResources/simple-example).
 
The option `-reporters=Elements` enables reporting to Mutation Testing Elements,
the optional option `-report-dir` and `-report-name` specifies the output path.

**Note:** Reporting to Mutation Testing Elements requires presence of the 
`-compdb-path` and `-compilation-flags` (see Junk Mutations above) because Mull 
needs the compilation database/flags information to reason about the source code 
in which the mutations are found.

```
mull-cxx --help
...
  -elements-output-path=<string>   - Where to store the Mutation Testing Elements JSON report (defaults to /tmp/mull.mutation-testing-elements.json)
  -reporter-elements               - Enables Mutation Testing Elements Reporter
...
``` 
 
### Examples: OpenSSL

Build OpenSSL:

```
git clone https://github.com/openssl/openssl.git
cd openssl
./config -no-asm -no-shared -g -O0 -fembed-bitcode
make all
```

Run without junk detection:

```
mull-cxx -test-framework=CustomTest test/bio_enc_test
```

Run with junk detection:

```
mull-cxx -test-framework=CustomTest \
  -compilation-flags="\
    -I . -I crypto/modes -I crypto/include -I include -I apps \
    -isystem /usr/local/include \
    -isystem /opt/llvm/5.0.0/lib/clang/5.0.0/include \
    -isystem /usr/include" \
  test/bio_enc_test
```

Note, the `isystem` header search paths are the implicit paths used by Clang,
that we should add explicitly. These are platform/version dependent.


Upon success you should see similar output:

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

### Examples: fmtlib

Build fmtlib:

```
git clone https://github.com/fmtlib/fmt.git
cd fmt
mkdir build.dir
cd build.dir
cmake \
  -DCMAKE_CXX_FLAGS="-fembed-bitcode -g -O0" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make core-test
```

Run without junk detection and only one mutator:

```
mull-cxx -test-framework=GoogleTest -mutators=cxx_add_to_sub ./bin/core-test
```

Run with junk detection:

```
mull-cxx -test-framework=GoogleTest \
  -mutators=cxx_add_to_sub \
  -compdb-path compile_commands.json \
  -compilation-flags="\
    -isystem /opt/llvm/5.0.0/include/c++/v1 \
    -isystem /opt/llvm/5.0.0/lib/clang/5.0.0/include \
    -isystem /usr/include" \
    ./bin/core-test
```

You should get the following output:

```
Extracting bitcode from executable (threads: 1): 1/1. Finished in 218ms.
Loading bitcode files (threads: 4): 4/4. Finished in 373ms.
Compiling instrumented code (threads: 4): 4/4. Finished in 12ms.
Loading dynamic libraries (threads: 1): 1/1. Finished in 0ms.
Searching tests (threads: 1): 1/1. Finished in 1ms.
Preparing original test run (threads: 1): 1/1. Finished in 68ms.
Running original tests (threads: 8): 29/29. Finished in 71ms.
Searching mutants across functions (threads: 8): 2630/2630. Finished in 64ms.
Filtering out junk mutations (threads: 8): 157/157. Finished in 3313ms.
Preparing mutations (threads: 1): 1/1. Finished in 1076ms.
Applying mutations (threads: 1): 26/26. Finished in 12ms.
Compiling original code (threads: 4): 4/4. Finished in 12ms.
Running mutants (threads: 8): 26/26. Finished in 805ms.

Survived mutants (18/26):

/tmp/1553947865/fmt/test/gmock-gtest-all.cc:9758:53: warning: Math Add: replaced + with -
    const int actual_to_skip = stack_frames_to_skip + 1;
                                                    ^
/tmp/1553947865/fmt/include/fmt/core.h:309:19: warning: Math Add: replaced + with -
    reserve(size_ + 1);
                  ^
/tmp/1553947865/fmt/test/core-test.cc:416:45: warning: Math Add: replaced + with -
    for (std::size_t j = 0; j < num_inputs; ++j) {
                                            ^
/tmp/1553947865/fmt/test/core-test.cc:415:43: warning: Math Add: replaced + with -
  for (std::size_t i = 0; i < num_inputs; ++i) {
                                          ^
/tmp/1553947865/fmt/test/core-test.cc:416:45: warning: Math Add: replaced + with -
    for (std::size_t j = 0; j < num_inputs; ++j) {
                                            ^
/tmp/1553947865/fmt/test/core-test.cc:415:43: warning: Math Add: replaced + with -
  for (std::size_t i = 0; i < num_inputs; ++i) {
                                          ^
/tmp/1553947865/fmt/test/core-test.cc:416:45: warning: Math Add: replaced + with -
    for (std::size_t j = 0; j < num_inputs; ++j) {
                                            ^
/tmp/1553947865/fmt/test/core-test.cc:415:43: warning: Math Add: replaced + with -
  for (std::size_t i = 0; i < num_inputs; ++i) {
                                          ^
/tmp/1553947865/fmt/test/core-test.cc:416:45: warning: Math Add: replaced + with -
    for (std::size_t j = 0; j < num_inputs; ++j) {
                                            ^
/tmp/1553947865/fmt/test/core-test.cc:415:43: warning: Math Add: replaced + with -
  for (std::size_t i = 0; i < num_inputs; ++i) {
                                          ^
/tmp/1553947865/fmt/test/core-test.cc:416:45: warning: Math Add: replaced + with -
    for (std::size_t j = 0; j < num_inputs; ++j) {
                                            ^
/tmp/1553947865/fmt/test/core-test.cc:415:43: warning: Math Add: replaced + with -
  for (std::size_t i = 0; i < num_inputs; ++i) {
                                          ^
/tmp/1553947865/fmt/test/core-test.cc:416:45: warning: Math Add: replaced + with -
    for (std::size_t j = 0; j < num_inputs; ++j) {
                                            ^
/tmp/1553947865/fmt/test/core-test.cc:415:43: warning: Math Add: replaced + with -
  for (std::size_t i = 0; i < num_inputs; ++i) {
                                          ^
/tmp/1553947865/fmt/include/fmt/format.h:1086:67: warning: Math Add: replaced + with -
  if (next_arg_id_ >= 0) return internal::to_unsigned(next_arg_id_++);
                                                                  ^
/tmp/1553947865/fmt/include/fmt/format.h:933:54: warning: Math Add: replaced + with -
    *--buffer = static_cast<Char>(data::DIGITS[index + 1]);
                                                     ^
/tmp/1553947865/fmt/include/fmt/format.h:939:39: warning: Math Add: replaced + with -
    *--buffer = static_cast<Char>('0' + value);
                                      ^
/tmp/1553947865/fmt/include/fmt/format.h:1086:67: warning: Math Add: replaced + with -
  if (next_arg_id_ >= 0) return internal::to_unsigned(next_arg_id_++);
                                                                  ^
Mutation score: 30%

Total execution time: 6072ms
```

## Contributing

Here is the starting point: [CONTRIBUTING.md](CONTRIBUTING.md)

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

Copyright (c) 2016-2020 Alex Denisov <alex@lowlevelbits.org> and Stanislav Pankevich <s.pankevich@gmail.com>. See LICENSE for details.
