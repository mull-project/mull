Non-standard test suites
========================

The goal of this tutorial is to demonstrate how to use Mull with 'non-standard'
test suites, such as when the test suite is a separate program. The best example
is integration tests written in interpreted languages.

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

Usage of Mull in this case is very similar to a "typical" use-case (see :doc:`Hello World tutorial <./HelloWorld>`).

1. Create config file ``mull.yml``:

.. code-block:: yaml

    mutators:
     - cxx_add_to_sub
     - cxx_mul_to_div

2. Generate mutated executable

.. code-block:: text

    $ clang-12 -fexperimental-new-pass-manager \
      -fpass-plugin=/usr/local/lib/mull-ir-frontend-12 \
      -g -grecord-command-line \
      main.c -o test.exe

3. Run analysis using `mull-runner`:

.. code-block:: shell

    $ mull-runner-12 ./test.exe -ide-reporter-show-killed \
      -test-program=python3 -- test.py ./test.exe
    [info] Using config /tmp/sc-kGN35Gr1f/mull.yml
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 347ms
    [info] Filter mutants (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 76ms
    [info] Running mutants (threads: 2)
           [################################] 2/2. Finished in 81ms
    [info] Killed mutants (2/2):
    /tmp/sc-kGN35Gr1f/main.c:5:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
      return a + b;
               ^
    /tmp/sc-kGN35Gr1f/main.c:9:12: warning: Killed: Replaced * with / [cxx_mul_to_div]
      return a * b;
               ^
    [info] All mutations have been killed
    [info] Mutation score: 100%
    [info] Total execution time: 509ms

Note, `test.exe` appears twice in the arguments list: the first appearance
is required for `mull-runner` to extract the mutants generated at the second step.
The second appearance is passed as an argument to the test program ``test.py``.
