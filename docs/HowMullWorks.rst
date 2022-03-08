How Mull works
==============

This page contains a short summary of the design and features of Mull. Also
the advantages of Mull are highlighted as well as some known issues.

If you want to learn more than we cover here, Mull has a paper:
"Mull it over: mutation testing based on LLVM" (see below on this page).

----

Design
------

Mull is based on LLVM and uses its API extensively. The main APIs used are:
**LLVM IR** and **Clang AST API**.

Mull finds and creates mutations of a program in memory, on the level of LLVM
bitcode.

All mutations are injected into original program's code. Each injected mutation
is hidden under a conditional flag that enables that specific mutation. The
resulting program is compiled into a single binary which is run multiple times,
one run per mutation. With each run, Mull activates a condition for a
corresponding mutation to check how the injection of that particular mutation
affects the execution of a test suite.

Mull runs the tested program and its mutated versions in child subprocesses so
that the execution of the tested program does not affect Mull running in a
parent process.

**Note:** Mull no longer uses LLVM JIT for execution of mutated programs.
See the
`Historical note: LLVM JIT deprecation (January 2021)`_.

Mull uses information about source code obtained via Clang AST API to find out
which mutations in LLVM bitcode are valid (i.e. they trace back to the source
code), all invalid mutations are ignored in a controlled way.

Mutations search
----------------

The default search algorithm simply finds all mutations that can be found on the
level of LLVM bitcode.

The **"IR search" algorithm** called Junk Detection uses source code information
provided by Clang AST to filter out invalid mutations from a set of all possible
mutations that are found in LLVM IR by the default search algorithm.

The **"AST search" algorithm** starts with collecting source code information
via Clang AST and then feeds this information to the default search algorithm
which allows finding valid mutations and filtering out invalid mutations
at the same time.

The IR and AST search algorithms are very similar in the reasoning that
they do. The only difference is that the AST search filters out invalid
mutations just in time as they are found in LLVM bitcode, while the IR search
does this after the fact on the raw set of mutations that consists of both valid
and invalid mutations.

The IR search algorithm appeared earlier and is expected to be more
stable. The AST search algorithm is currently in development.

Supported mutation operators
----------------------------

See `Supported Mutation Operators <SupportedMutations.html>`_.

Reporting
---------

Mull reports survived/killed mutations to the console by default. The
compiler-like warnings are printed to standard output.

Mull has an SQLite reporter: mutants and execution results are collected in
SQLite database. This kind of reporting makes it possible to make SQL queries
for a more advanced analysis of mutation results.

Mull supports reporting to HTML via
`Mutation Testing Elements <https://github.com/stryker-mutator/mutation-testing-elements>`_. Mull generates JSON report which is given to Elements to generate HTML pages.

Platform support
----------------

Mull has a great support of macOS and various Linux systems across all modern
versions of LLVM from 9.0 to 13.0. All the new versions of LLVM are supported as
soon as they released.

Mull is reported to work on Windows Subsystem for Linux, but no official support
yet.

Test coverage
-------------

Mull has 3 layers of testing:

1. Unit and integration testing on the level of C++ classes
2. Integration testing against known real-world projects, such as OpenSSL
3. Integration testing using LLVM Integrated Tester (LIT)

Advantages
----------

The main advantage of Mull's design and its approach to finding and doing
mutations is very good performance. Combined with incremental mutation testing
one can get mutation testing reports in the order of few seconds.

Another advantage is language agnosticism. The developers of Mull have been
focusing on C/C++ as the primary supported languages but the proof of concepts
for other compiled languages, such as Rust and Swift, have been developed.

A lot of development effort have been put into Mull in order to make it stable
across different operating systems and versions of LLVM. Combined with the
growing test coverage and highly modular design, Mull is a very stable,
well-tested and maintained system.

Known issue: Precision
----------------------

Mull works on the level of LLVM bitcode and from there it gets its strengths
but also its main weakness: the precision of the information for mutations is
not as high as it is on the source code level. It is a broad area of work where
the developers of Mull have to combine the two levels of information about code:
LLVM bitcode and AST in order to make Mull both fast and precise. Among other
things the good suite of integration tests is aimed to provide Mull with a good
contract of supported mutations which are predictable and known to work without
any side effects.

Historical note: LLVM JIT deprecation (January 2021)
----------------------------------------------------

The usage of LLVM JIT has been deprecated and all LLVM JIT-related code has been
removed from Mull by January 2021.

This issue explains the reasons:
`PSA: Moving away from JIT <https://github.com/mull-project/mull/issues/798>`_.

Paper
-----

`Mull it over: mutation testing based on LLVM (preprint) <https://lowlevelbits.org/pdfs/Mull_Mutation_2018.pdf>`_

.. code-block:: text

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

Additional information about Mull
---------------------------------

- `2019 EuroLLVM Developers’ Meeting: A. Denisov “Building an LLVM-based tool: lessons learned" <https://www.youtube.com/watch?v=Yvj4G9B6pcU>`_ and blog post
  `Building an LLVM-based tool. Lessons learned <https://lowlevelbits.org/
  building-an-llvm-based-tool.-lessons-learned/>`_

- `Mutation Testing: implementation details <https://lowlevelbits.org/mutation-testing-implementation-details/>`_

- `Mutation testing for Swift with Mull: how it could work. Looking for contributors <https://stanislaw.github.io/2018/09/03/mull-and-swift-how-it-almost-works.html>`_

- `Mull meets Rust (LLVM Social Berlin #6, 23.02.2017) <https://www.youtube.com/watch?v=VasSufnFswc&feature=youtu.be>`_
