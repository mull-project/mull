Non-standard test suites
========================

.. warning::
   Since the version 0.15.0, ``mull-cxx`` tool `is deprecated <https://github.com/mull-project/mull/issues/945>`_
   in favour of a compiler plugin: `Mull IR Frontend <https://github.com/mull-project/mull/pull/938>`_.

   This tutorial covers ``mull-cxx`` tool and will be updated in the future.


The goal of this tutorial is to demonstrate how to use Mull with 'non-standard'
test suites, such as when the test suite is a separate program. The best example
is integration tests written in interpreted languages.

Two-step analysis process
-------------------------

The typical process of applying Mull is a one-step action: run `mull-cxx` and wait
for the results. Here is what `mull-cxx` does under the hood:

1. Generates a mutated version of the original program
2. Runs all the mutants
3. Generates report(s)

Since version 0.11.0, there is a way to split this process into a two-step
action: run `mull-cxx` to generate mutated program, and then run `mull-runner`
to assess all the mutants and generate reports.

Given the program from the `Hello World Example <HelloWorld.html>`_ the following
two runs are identical:

One-step process:

.. code-block:: shell

    $ clang -fembed-bitcode -g main.cpp -o hello-world
    $ mull-cxx -ide-reporter-show-killed hello-world

Two-step process:

.. code-block:: shell

    $ clang -fembed-bitcode -g main.cpp -o hello-world
    $ mull-cxx -mutate-only -output=hello-world-mutated hello-world
    $ mull-runner -ide-reporter-show-killed hello-world-mutated

While this is useful, let's look into a slightly more complex example.

Tests in interpreted languages
------------------------------

Consider the following (absolutely synthetic) program under test:

.. code-block:: c

    extern int printf(const char *, ...);
    extern int strcmp(const char *, const char *);

    int test1(int a, int b) {
      return a + b;
    }

    int test2(int a, int b) {
      return a * b;
    }

    int main(int argc, char **argv) {
      if (argc == 1) {
        printf("NOT ENOUGH ARGUMENTS\n");
        return 1;
      }
      if (strcmp(argv[1], "first test") == 0) {
        if (test1(2, 5) == 7) {
          printf("first test passed\n");
          return 0;
        } else {
          printf("first test failed\n");
          return 1;
        }
      } else if (strcmp(argv[1], "second test") == 0) {
        if (test2(2, 5) == 10) {
          printf("second test passed\n");
          return 0;
        } else {
          printf("second test failed\n");
          return 1;
        }
      } else {
        printf("INCORRECT TEST NAME %s\n", argv[1]);
        return 1;
      }
      return 0;
    }

The program accepts a command-line argument, and depending on the value of the
argument it either runs one of the tests or exists with an error.
Here is an example:

.. code-block:: shell

    $ clang main.c -o test
    $ ./test
    NOT ENOUGH ARGUMENTS
    $ ./test "first test"
    first test passed
    $ ./test "second test"
    second test passed
    $ ./test "third test"
    INCORRECT TEST NAME third test

Running these tests manually is a tedious and error-prone process, so we create
a separate test runner:

.. code-block:: python

    import sys
    import subprocess

    test_executable = sys.argv[1]

    subprocess.run([test_executable, "first test"], check=True)
    subprocess.run([test_executable, "second test"], check=True)

The script takes the program under test as its argument and runs the tests against
that program.

.. code-block:: shell

    $ clang main.c -o test
    $ python3 test.py ./test
    first test passed
    second test passed

In this case, simply using `mull-cxx` is not enough: Mull doesn't know how to run
the "external" test suite (`test.py`), so we must be using `mull-runner` for this.
The process is two-step.

1. Generate mutated executable

.. code-block:: text

    $ clang -fembed-bitcode -g main.c -o test
    $ mull-cxx -mutate-only \
      -mutators=cxx_add_to_sub -mutators=cxx_mul_to_div \
      -output=test.mutated ./test
    [info] Mutate-only mode on: Mull will generate mutants, but won't run them
    ...
    [info] Mutated executable: test.mutated
    [info] Total execution time: 182ms

2. Run analysis using `mull-runner`:

.. code-block:: shell

    $ mull-runner test.mutated -ide-reporter-show-killed \
      -test-program=python3 -- test.py test.mutated
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 398ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 60ms
    [info] Running mutants (threads: 2)
           [################################] 2/2. Finished in 76ms
    [info] Killed mutants (2/2):
    main.c:5:16: warning: Killed: Replaced + with - [cxx_add_to_sub]
          return a + b;
                   ^
    main.c:9:16: warning: Killed: Replaced * with / [cxx_mul_to_div]
          return a * b;
                   ^
    [info] All mutations have been killed
    [info] Mutation score: 100%
    [info] Total execution time: 535ms

Note, `test.mutated` appears twice in the arguments list: the first appearance
is required for `mull-runner` to extract the mutants generated at the first step.
The second appearance is passed to the test program.
