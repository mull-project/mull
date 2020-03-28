Hello World Example
===================

The goal of this tutorial is to demonstrate how to run Mull on minimal C
programs. After reading it you should have a basic understanding of what
arguments Mull needs in order to create mutations in your programs, run the
mutants and generate mutation testing reports.

**TL;DR version**: if you want to run a single copy and paste example, scroll
down to ``Complete example`` below.

----

The tutorial assumes that you have `installed <Installation.html>`_ Mull on your system and
have the `mull-cxx` executable available:

.. code-block:: bash

    $ mull-cxx -version
    Mull: LLVM-based mutation testing
    https://github.com/mull-project/mull
    Version: 0.7.0
    Commit: 1638698
    Date: 28 Mar 2020
    LLVM: 9.0.0

The most important thing that Mull needs to know is the path to your program
which must be a valid C or C++ executable. Let's create a C program:

.. code-block:: c

    int main() {
      return 0;
    }

and compile it:

.. code-block:: bash

    clang main.cpp -o hello-world

We can already try running ``mull-cxx`` and see what happens:

.. code-block:: bash

    $ mull-cxx hello-world
    mull-cxx: for the -test-framework option: must be specified at least once!

This is the second important thing that Mull needs: we have to specify which
kind of test framework Mull should assume our program uses.

We specify ``CustomTest``:

.. code-block:: bash

    mull-cxx -test-framework=CustomTest hello-world

``-test-framework=CustomTest`` parameter tells Mull that it should not expect
a real test framework such as Google Test or any kind of advanced test suite.
Instead Mull will simply consider that our tests will be simple test functions
which we will call from the ``main()`` function.

Now the output is different:

.. code-block:: text

    [info] Extracting bitcode from executable (threads: 1)
    [warning] No bitcode: x86_64
           [################################] 1/1. Finished in 1ms
    [info] Loading dynamic libraries (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Searching tests (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] No mutants found. Mutation score: infinitely high

Notice the ``No bitcode: x86_64`` warning! Now Mull is already trying to work
with our executable but there is still one important detail that is missing: we
haven't compiled the program with a special option that embeds LLVM bitcode
into our executable.

Mull works on a level of LLVM Bitcode relying on debug information to show
results, therefore you should build your project with ``-fembed-bitcode`` and
``-g`` flags enabled.

Let's try again:

.. code-block:: text

    $ clang -fembed-bitcode -g main.cpp -o hello-world
    $ mull-cxx -test-framework CustomTest hello-world
    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 3ms
    [info] Loading bitcode files (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Compiling instrumented code (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Loading dynamic libraries (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Searching tests (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Preparing original test run (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Running original tests (threads: 1)
           [################################] 1/1. Finished in 12ms
    [info] No mutants found. Mutation score: infinitely high

The ``No bitcode: x86_64`` warning has gone and now we can focus on another
important part of the output: ``No mutants found. Mutation score: infinitely
high``. We have our executable but we don't have any code so there is nothing
Mull could work on.

Let's add some code:

.. code-block:: c

    bool valid_age(int age) {
      if (age >= 21) {
        return true;
      }
      return false;
    }

    int main() {
      int test1 = valid_age(25) == true;
      if (!test1) {
        /// test failed
        return 1;
      }

      int test2 = valid_age(20) == false;
      if (!test2) {
        /// test failed
        return 1;
      }

      /// success
      return 0;
    }

We compile this new code using the bitcode flags and run the Mull again. This
time we also want to add additional flag ``-ide-reporter-show-killed`` which
tells Mull to print killed mutations. Normally we are not interested in seeing
killed mutations in console input but in this tutorial we want to be more
verbose.

.. code-block:: text

    $ clang -fembed-bitcode -g main.cpp -o hello-world
    $ mull-cxx -test-framework=CustomTest -ide-reporter-show-killed hello-world
    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Loading bitcode files (threads: 1)
           [################################] 1/1. Finished in 12ms
    [info] Compiling instrumented code (threads: 1)
           [################################] 1/1. Finished in 12ms
    [info] Loading dynamic libraries (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Searching tests (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Preparing original test run (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Running original tests (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Applying function filter: no debug info (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Applying function filter: file path (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Instruction selection (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Searching mutants across functions (threads: 1)
           [################################] 1/1. Finished in 13ms
    [info] Applying filter: no debug info (threads: 2)
           [################################] 2/2. Finished in 0ms
    [info] Applying filter: file path (threads: 2)
           [################################] 2/2. Finished in 10ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Cloning functions for mutation (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Removing original functions (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Redirect mutated functions (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Applying mutations (threads: 1)
           [################################] 2/2. Finished in 0ms
    [info] Compiling original code (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Running mutants (threads: 2)
           [################################] 2/2. Finished in 12ms
    [info] Killed mutants (1/2):
    /tmp/sc-b3yQyijWP/main.cpp:2:11: warning: Killed: Replaced >= with < [cxx_ge_to_lt]
      if (age >= 21) {
              ^
    [info] Survived mutants (1/2):
    /tmp/sc-b3yQyijWP/main.cpp:2:11: warning: Survived: Replaced >= with > [cxx_ge_to_gt]
      if (age >= 21) {
              ^
    [info] Mutation score: 50%

What we are seeing now is two mutations: one mutation is ``Killed``, another
one is ``Survived``. If we take a closer look at the code and the contents
of the tests ``test1`` and ``test2`` we will see that one important test case
is missing: the one that would test the age ``21`` and this is exactly
what the survived mutation is about: Mull has replaced ``age >= 21`` with
``age > 21`` and neither of the two tests have detected the mutation.

Let's add the third test case and see what happens.

Complete example
----------------

The code:

.. code-block:: cpp

    bool valid_age(int age) {
      if (age >= 21) {
        return true;
      }
      return false;
    }

    int main() {
      bool test1 = valid_age(25) == true;
      if (!test1) {
        /// test failed
        return 1;
      }

      bool test2 = valid_age(20) == false;
      if (!test2) {
        /// test failed
        return 1;
      }

      bool test3 = valid_age(21) == true;
      if (!test3) {
         /// test failed
         return 1;
      }

      /// success
      return 0;
    }

.. code-block:: text

    $ clang -fembed-bitcode -g main.cpp -o hello-world
    $ mull-cxx -test-framework=CustomTest -ide-reporter-show-killed hello-world
    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 4ms
    [info] Loading bitcode files (threads: 1)
           [################################] 1/1. Finished in 12ms
    [info] Compiling instrumented code (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Loading dynamic libraries (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Searching tests (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Preparing original test run (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Running original tests (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Applying function filter: no debug info (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Applying function filter: file path (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Instruction selection (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Searching mutants across functions (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Applying filter: no debug info (threads: 2)
           [################################] 2/2. Finished in 0ms
    [info] Applying filter: file path (threads: 2)
           [################################] 2/2. Finished in 0ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Cloning functions for mutation (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Removing original functions (threads: 1)
           [################################] 1/1. Finished in 12ms
    [info] Redirect mutated functions (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Applying mutations (threads: 1)
           [################################] 2/2. Finished in 0ms
    [info] Compiling original code (threads: 1)
           [################################] 1/1. Finished in 12ms
    [info] Running mutants (threads: 2)
           [################################] 2/2. Finished in 10ms
    [info] Killed mutants (2/2):
    /tmp/sc-b3yQyijWP/main.cpp:2:11: warning: Killed: Replaced >= with > [cxx_ge_to_gt]
      if (age >= 21) {
              ^
    /tmp/sc-b3yQyijWP/main.cpp:2:11: warning: Killed: Replaced >= with < [cxx_ge_to_lt]
      if (age >= 21) {
              ^
    [info] All mutations have been killed
    [info] Mutation score: 100%

Summary
-------

This is a short summary of what we have learned in tutorial:

- Your code has to be compiled with ``-fembed-bitcode -g`` compile flags:

  - Mull expects embedded bitcode files to be present in binary executable
    (ensured by ``-fembed-bitcode``).

  - Mull needs debug information to be included by the compiler (enabled by
    ``-g``). Mull uses this information to find mutations in bitcode and source
    code.

- Mull expects the following arguments to be always provided:

  - Your executable program

  - ``-test-framework`` parameter that tells Mull which kind of testing
    framework to expect. In this tutorial we have been using the ``CustomTest``
    framework.