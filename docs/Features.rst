Features
========

- Mull enables mutation testing of C and C++ projects.

- Mull expects a passing test suite to exit with 0. If the test suite fails, it
  must exit with a non-zero exit code. Any C/C++ test framework that follows
  this convention is supported by Mull.

- `Supported Mutations <SupportedMutations.html>`_.

- Generate results in various formats:

  - IDE Reporter: compiler-like warnings are printed to standard output

    - ``sample.cpp:15:11: warning: Killed: Replaced >= with < [cxx_ge_to_lt]``

  - SQLite Reporter: SQLite database file.

  - JSON file that conforms `mutation-testing-elements schema <https://github.com/stryker-mutator/mutation-testing-elements/tree/master/packages/report-schema>`_

  - `Mutation Testing Elements HTML Reporter <https://github.com/stryker-mutator/mutation-testing-elements/tree/master/packages/elements>`_

- Parallelized execution of tests

- `Incremental mutation testing <IncrementalMutationTesting.html>`_.
  Working with mutations found in Git Diff changesets.

- Mull requires test programs to be compiled with Clang/LLVM. Mull supports
  all LLVM versions starting from LLVM 9.

For a more detailed description of Mull's architecture, see
`How Mull works <HowMullWorks.html>`_.
