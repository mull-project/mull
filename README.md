# Mull

![Screenshot](/screenshot.png)


See [LLVM-based Mutation Testing System. Request For
Comments](http://lowlevelbits.org/llvm-based-mutation-testing-system/) and [FOSDEM 2017: Mutation Testing, Leaving the Stone Age by Alex Denisov](https://www.youtube.com/watch?v=YEgiyiICkpQ).

[![Slack Status](https://mutation-testing-slack.herokuapp.com/badge.svg)](https://mutation-testing.slack.com/messages/mull)

## Getting started

Mull is being developed and tested against macOS and Linux Ubuntu.
Also it is known to work against CentOS 7.

 Currently supported LLVM versions: 3.9, 4.0, 5.0, 6.0.

[How to run Mull](Docs/HowToRunMull.md)

## Notes

Use `make help` to see the common tasks used by developers.

Use `MullUnitTests` scheme to run the tests. Use `mull-driver` scheme to
build Mull.

## Supported mutation operators

- **Math: Add,Sub,Mul,Div**: replaces + with -, - with +, * with /, / with *.
- **Negate Condition**: inverses conditions like `A -> !A` or `== -> !=`.
- **Remove Void Function Call**: removes a function which doesn't have a return value.
- **Replace Call**: replaces function calls with Int, Float and Double return values with number 42. Alpha state, not enabled by default.
- **Scalar Value**: replaces 0 with 1, other numbers with 0. Alpha state, not enabled by default.
- **AND <-> OR**: replaces `&& -> ||` and vice versa. Alpha state, not enabled by default.

## Open questions

There are the questions we want to answer with Mull eventually.

- Can mutation testing actually detect serious errors in the real-world
programs?
- What are the most effective mutation operators?
- Is there any equivalence between mutation testing and different kinds of
testing coverage especially MC/DC coverage?
- Can mutation testing be combined with other techniques like symbolic
execution, fuzz testing, genetic programming, etc., to help in automatic test
generation?

## Useful Material

### Papers

[Mull it over: mutation testing based on LLVM (preprint)](https://lowlevelbits.org/pdfs/Mull_Mutation_2018.pdf)

[An Analysis and Survey of the Development of
Mutation Testing](http://www0.cs.ucl.ac.uk/staff/mharman/tse-mutation-survey.pdf)

[Are Mutants a Valid Substitute for Real Faults in Software Testing?](https://homes.cs.washington.edu/~mernst/pubs/mutation-effectiveness-fse2014.pdf)

### Articles

[LLVM-based Mutation Testing System. Request For
Comments](http://lowlevelbits.org/llvm-based-mutation-testing-system/)

[Mull and Klee, part 1: mutation testing analysis for Klee's Tutorial Two](http://stanislaw.github.io/2017/05/07/mull-and-klee-part1.html)

[Using mutation testing score to measure a reliability of a single function](http://stanislaw.github.io/2017/05/05/unbreakable-code.html)

### Stack Exchange

[Mutation testing: is it useful for assessing test quality?](https://sqa.stackexchange.com/questions/5255/mutation-testing-is-it-useful-for-assessing-test-quality)

### Talks

[FOSDEM 2017: Mutation Testing, Leaving the Stone Age by Alex Denisov](https://www.youtube.com/watch?v=YEgiyiICkpQ)

[LLVM Social Berlin #6: Mull meets Rust](https://www.youtube.com/watch?v=VasSufnFswc)

## Get in touch

The authors and developers of Mull are Alex Denisov 1101.debian@gmail.com and Stanislav Pankevich s.pankevich@gmail.com.

Feel free to contact us if you have any questions about Mull and mutation testing or any other topics related to software reliability and software in general.

## Citation

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

Copyright (c) 2018 Alex Denisov and Stanislav Pankevich. See LICENSE for details.

