# Mull

![Screenshot](/screenshot.png)


See [LLVM-based Mutation Testing System. Request For
Comments](http://lowlevelbits.org/llvm-based-mutation-testing-system/) and [FOSDEM 2017: Mutation Testing, Leaving the Stone Age by Alex Denisov](https://www.youtube.com/watch?v=YEgiyiICkpQ).

[![Slack Status](https://mutation-testing-slack.herokuapp.com/badge.svg)](https://mutation-testing.slack.com/messages/mull)

## Getting started

Mull is being developed against OSX. Also it is known to work against
Linux Ubuntu 16.04 and CentOS 7.

- [Getting started on Mac OS](/Docs/GettingStartedMacOS.md)
- [Getting started on Ubuntu](/Docs/GettingStartedUbuntu.md)
- [Getting started on CentOS](/Docs/GettingStartedCentOS7.md)

## Notes

Use `make -f Makefile.macos help` to see the common tasks used by developers.

Use `MullUnitTests` scheme to run the tests. Use `mull-driver` scheme to
build Mull.

## Supported mutation operators

- **Math: Add,Sub,Mul,Div**: replaces + with -, - with +, * with /, / with *.
- **Negate Condition**: inverses conditions like `A -> !A` or `== -> !=`.
- **Remove Void Function Call**: removes a function which doesn't have a return value.
- **Scalar Value**: replaces 0 with 1, other numbers with 0. Alpha state, not enabled by default.
- **AND <-> OR**: replaces `&& -> ||` and vice versa. Alpha state, not enabled by default.

## Open questions

There are the questions we want to answer with Mull eventually.

- Can mutation testing actually detect serious errors in the real-world programs?
- What are the most effective mutation operators?
- Is there any equivalence between mutation testing and different kinds of testing coverage especially MC/DC coverage?

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

## Get in touch

The authors and developers of Mull are Alex Denisov 1101.debian@gmail.com and Stanislav Pankevich s.pankevich@gmail.com.

Feel free to contact us if you have any questions about Mull and mutation testing or any other topics related to software reliability and software in general.
