For researchers
===============

This page contains a short summary of the design and features of Mull. Also
the advantages of Mull are highlighted as well as some known issues.

If you want to learn more than we cover here, Mull has a paper:
"Mull it over: mutation testing based on LLVM" (see below on this page).

----


Design
------

Mull is based on LLVM and uses its API extensively. The main APIs used are:
**LLVM IR**, **LLVM JIT**, **Clang AST API**.

Mull finds and creates mutations of a program in memory, on the level of LLVM
bitcode.

Mull uses information about source code obtained via Clang AST API to find which
mutations in LLVM bitcode are valid (i.e. they trace back to the source code),
all invalid mutations are ignored in a controlled way.

Mull runs the program and its mutated versions in memory using LLVM JIT. The
``fork()`` call is used to run mutants in child subprocesses so that their
execution does not affect Mull as a parent process.

Mutations search
----------------

The default search algorithm simply finds all mutations that can be made on the
level of LLVM bitcode.

The **"black search" algorithm** called Junk Detection uses source code information
provided by Clang AST to filter out invalid mutations from a set of all possible
mutations that are found in LLVM bitcode by the default search algorithm.

The **"white search"** algorithm starts with collecting source code information
via Clang AST and then feeds this information to the default search algorithm
which allows finding valid mutations and filtering out invalid mutations
at the same time.

The black and white search algorithms are very similar in the reasoning that
they do. The only difference is that the white search filters out invalid
mutations just in time as they are found in LLVM bitcode, while the black search
does this after the fact on the raw set of mutations that consists of both valid
and invalid mutations.

The black search algorithm appeared earlier and is expected to be more
stable. The white search algorithm is currently in development.

Supported mutation operators
----------------------------

See `Supported Mutation Operators <SupportedMutations.html>`_.

Reporting
---------

Mull reports survived/killed mutations to the console by default.

Mull has an SQLite reporter: mutants and execution results are collected in
SQLite database. This kind of reporting makes it possible to make SQL queries
for a more advanced analysis of mutation results.

Mull supports reporting to HTML via
`Mutation Testing Elements <https://github.com/stryker-mutator/mutation-testing-elements>`_. Mull generates JSON report which is given to Elements to generate HTML pages.

Platform support
----------------

Mull has a great support of macOS and various Linux systems across all modern
versions of LLVM from 3.9.0 to 9.0.0.

Mull supports FreeBSD with minor known issues.

Mull is reported to work on Windows Subsystem for Linux, but no official support
yet.

Test coverage
-------------

Mull has 3 layers of testing:

1. Unit and integration testing on the level of C++ classes
2. Integration testing against known real-world projects, such as OpenSSL
3. Integration testing using LLVM Integrated Tester (in progress)

Current development
-------------------

The current development goals for Mull for Autumn 2019 - Spring 2020 are:

- Stable performance of black and white search algorithms supported by a solid
  integration test coverage.

- **Incremental mutation testing**. Mull can already run on subsets of program
  code but the API and workflows are still evolving.

- More mutation operators.

Advantages
----------

The main advantage of Mull's design and its approach to finding and doing
mutations is very good performance. Combined with incremental mutation testing
one can get mutation testing reports in the order of few seconds.

Another advantage is language agnosticism. The developers of Mull have been
focusing on C/C++ as their primary development languages at their jobs but
the proof of concepts have been developed for the other compiled languages such
as Rust and Swift.

A lot of development effort have been put into Mull in order to make it stable
across different operating systems and versions of LLVM. Combined with the
growing test coverage and highly modular design the authors are slowly but
steady getting to the point when they can claim that Mull is a very stable,
very well tested and maintained system.

Known issues
------------

Mull works on the level of LLVM bitcode and from there it gets its strengths
but also its main weakness: the precision of the information for mutations is
not as high as it is on the source code level. It is a broad area of work where
the developers of Mull have to combine the two levels of information about code:
LLVM bitcode and AST in order to make Mull both fast and precise. Among other
things the good suite of integration tests is aimed to provide Mull with a good
contract of supported mutations which are predictable and known to work without
any side effects.

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

