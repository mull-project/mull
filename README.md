# Mull

See [LLVM-based Mutation Testing System. Request For
Comments](http://lowlevelbits.org/llvm-based-mutation-testing-system/) and [FOSDEM 2017: Mutation Testing, Leaving the Stone Age by Alex Denisov](https://www.youtube.com/watch?v=YEgiyiICkpQ).

[![Slack Status](https://mutation-testing-slack.herokuapp.com/badge.svg)](https://mutation-testing.slack.com/messages/mull)

## Getting started

So far Mull is being developed against OSX. Linux support is coming.

### OS X

**Warning:** Recently we migrated Mull to be an out-of-source project.
Migration went smoothly and all is working on our end (c). Please let us know
if you notice some dependencies that we missed to specify below.

#### Notes

Current code is being developed against stable LLVM 3.9 branch.

Either use binary distribution via `brew install llvm@3.9` or clone LLVM and
build it yourself from
[release_39](https://github.com/llvm-mirror/llvm/tree/release_39) branch.

#### Getting dependencies

```bash
# These are needed by LLVM
brew install zlib
brew install ncurses

# SQLite is needed for Mull to generate reports.
brew install sqlite
```

#### Getting Mull

```bash
git clone https://github.com/mull-project/mull.git
cd mull
```

#### Getting Google Test

After you clone Mull, you have to install Google Test.

```bash
cd googletest
make install
cd ..
```

#### Building Mull

```bash
mkdir BuildXcode
cd BuildXcode
cmake ../ -G Xcode -DCMAKE_TOOLCHAIN_FILE=../Mull.toolchain.OSX.cmake

open Mull.xcodeproj
```

Use `make help` to see the common tasks used by developers.

Use `MullUnitTests` scheme to run the tests. Use `mull-driver` scheme to
build Mull.

## Open questions

There are the questions we want to answer with Mull eventually.

- Can mutation testing actually detect serious errors in the real-world programs?
- What are the most effective mutation operators?

## Useful Material

### Papers

[An Analysis and Survey of the Development of
Mutation Testing](http://www0.cs.ucl.ac.uk/staff/mharman/tse-mutation-survey.pdf)

[Are Mutants a Valid Substitute for Real Faults in Software Testing?](https://homes.cs.washington.edu/~mernst/pubs/mutation-effectiveness-fse2014.pdf)

### Articles

[LLVM-based Mutation Testing System. Request For
Comments](http://lowlevelbits.org/llvm-based-mutation-testing-system/)

[Mull and Klee, part 1: mutation testing analysis for Klee's Tutorial Two](http://stanislaw.github.io/2017/05/07/mull-and-klee-part1.html)

[Using mutation testing score to measure a reliability of a single function](http://stanislaw.github.io/2017/05/05/unbreakable-code.html)

### Talks

[FOSDEM 2017: Mutation Testing, Leaving the Stone Age by Alex Denisov](https://www.youtube.com/watch?v=YEgiyiICkpQ)

[LLVM Social Berlin #6: Mull meets Rust](https://www.youtube.com/watch?v=VasSufnFswc)
