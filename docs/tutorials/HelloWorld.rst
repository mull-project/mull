Hello World Example
===================

The goal of this tutorial is to demonstrate how to run Mull on minimal C
programs. After reading it you should have a basic understanding of what
arguments Mull needs in order to create mutations in your programs, run the
mutants and generate mutation testing reports.

**TL;DR version**: if you want to run a single copy and paste example, scroll
down to ``Killing mutants again, all killed`` below.

----

Step 1: Checking version
------------------------

The tutorial assumes that you have `installed <Installation.html>`_ Mull on your system and
have the `mull-cxx` executable available:

.. code-block:: bash

    $ mull-cxx -version
    Mull: LLVM-based mutation testing
    https://github.com/mull-project/mull
    Version: 0.8.0
    Commit: f94f38ed
    Date: 04 Jan 2021
    LLVM: 9.0.0

Step 2: Enabling Bitcode
------------------------

The most important thing that Mull needs to know is the path to your program
which must be a valid C or C++ executable. Let's create a C program:

.. code-block:: c

    int main() {
      return 0;
    }

and compile it:

.. code-block:: bash

    $ clang main.cpp -o hello-world

We can already try running ``mull-cxx`` and see what happens:

.. code-block:: text

    $ mull-cxx hello-world
    [info] Extracting bitcode from executable (threads: 1)
    [warning] No bitcode: x86_64
           [################################] 1/1. Finished in 3ms
    [info] Sanity check run (threads: 1)
           [################################] 1/1. Finished in 409ms
    [info] Gathering functions under test (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] No mutants found. Mutation score: infinitely high
    [info] Total execution time: 413ms

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
    $ mull-cxx hello-world
    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 5ms
    [info] Loading bitcode files (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Sanity check run (threads: 1)
           [################################] 1/1. Finished in 336ms
    [info] Gathering functions under test (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Applying function filter: no debug info (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Applying function filter: file path (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Instruction selection (threads: 1)
           [################################] 1/1. Finished in 13ms
    [info] Searching mutants across functions (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] No mutants found. Mutation score: infinitely high
    [info] Total execution time: 400ms

The ``No bitcode: x86_64`` warning has gone and now we can focus on another
important part of the output: ``No mutants found. Mutation score: infinitely
high``. We have our executable but we don't have any code so there is nothing
Mull could work on.

Step 3: Killing mutants, one survived
-------------------------------------

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
killed mutations in console output but in this tutorial we want to be more
verbose.

.. code-block:: text

    $ clang -fembed-bitcode -g main.cpp -o hello-world
    $ mull-cxx -ide-reporter-show-killed hello-world
    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 6ms
    [info] Loading bitcode files (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Sanity check run (threads: 1)
           [################################] 1/1. Finished in 341ms
    [info] Gathering functions under test (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Applying function filter: no debug info (threads: 3)
           [################################] 3/3. Finished in 0ms
    [info] Applying function filter: file path (threads: 2)
           [################################] 2/2. Finished in 0ms
    [info] Instruction selection (threads: 2)
           [################################] 2/2. Finished in 11ms
    [info] Searching mutants across functions (threads: 2)
           [################################] 2/2. Finished in 10ms
    [info] Applying filter: no debug info (threads: 6)
           [################################] 6/6. Finished in 1ms
    [info] Applying filter: file path (threads: 6)
           [################################] 6/6. Finished in 0ms
    [info] Applying filter: junk (threads: 6)
           [################################] 6/6. Finished in 11ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Cloning functions for mutation (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Removing original functions (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Redirect mutated functions (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Applying mutations (threads: 1)
           [################################] 4/4. Finished in 12ms
    [info] Compiling original code (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Link mutated program (threads: 1)
           [################################] 1/1. Finished in 109ms
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 360ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 18ms
    [info] Running mutants (threads: 4)
           [################################] 4/4. Finished in 63ms
    [info] Killed mutants (3/4):
    /tmp/sc-PzmaCNIRu/main.cpp:2:15: warning: Killed: Replaced >= with < [cxx_ge_to_lt]
          if (age >= 21) {
                  ^
    /tmp/sc-PzmaCNIRu/main.cpp:9:33: warning: Killed: Replaced == with != [cxx_eq_to_ne]
          int test1 = valid_age(25) == true;
                                    ^
    /tmp/sc-PzmaCNIRu/main.cpp:15:33: warning: Killed: Replaced == with != [cxx_eq_to_ne]
          int test2 = valid_age(20) == false;
                                    ^
    [info] Survived mutants (1/4):
    /tmp/sc-PzmaCNIRu/main.cpp:2:15: warning: Survived: Replaced >= with > [cxx_ge_to_gt]
          if (age >= 21) {
                  ^
    [info] Mutation score: 75%
    [info] Total execution time: 996ms

What we are seeing now is four mutations: three mutations are ``Killed``, another
one is ``Survived``. If we take a closer look at the code and the contents
of the tests ``test1`` and ``test2`` we will see that one important test case
is missing: the one that would test the age ``21`` and this is exactly
what the survived mutation is about: Mull has replaced ``age >= 21`` with
``age > 21`` and neither of the two tests have detected the mutation.

Let's add the third test case and see what happens.

Step 4: Killing mutants again, all killed
-----------------------------------------

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
    $ mull-cxx -ide-reporter-show-killed hello-world
    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 4ms
    [info] Loading bitcode files (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Sanity check run (threads: 1)
           [################################] 1/1. Finished in 7ms
    [info] Gathering functions under test (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Applying function filter: no debug info (threads: 3)
           [################################] 3/3. Finished in 0ms
    [info] Applying function filter: file path (threads: 2)
           [################################] 2/2. Finished in 0ms
    [info] Instruction selection (threads: 2)
           [################################] 2/2. Finished in 12ms
    [info] Searching mutants across functions (threads: 2)
           [################################] 2/2. Finished in 10ms
    [info] Applying filter: no debug info (threads: 5)
           [################################] 5/5. Finished in 0ms
    [info] Applying filter: file path (threads: 5)
           [################################] 5/5. Finished in 1ms
    [info] Applying filter: junk (threads: 5)
           [################################] 5/5. Finished in 12ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Cloning functions for mutation (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Removing original functions (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Redirect mutated functions (threads: 1)
           [################################] 1/1. Finished in 10ms
    [info] Applying mutations (threads: 1)
           [################################] 5/5. Finished in 0ms
    [info] Compiling original code (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Link mutated program (threads: 1)
           [################################] 1/1. Finished in 62ms
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 311ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 19ms
    [info] Running mutants (threads: 5)
           [################################] 5/5. Finished in 63ms
    [info] Killed mutants (5/5):
    /tmp/sc-PzmaCNIRu/main.cpp:2:15: warning: Killed: Replaced >= with > [cxx_ge_to_gt]
          if (age >= 21) {
                  ^
    /tmp/sc-PzmaCNIRu/main.cpp:2:15: warning: Killed: Replaced >= with < [cxx_ge_to_lt]
          if (age >= 21) {
                  ^
    /tmp/sc-PzmaCNIRu/main.cpp:9:34: warning: Killed: Replaced == with != [cxx_eq_to_ne]
          bool test1 = valid_age(25) == true;
                                     ^
    /tmp/sc-PzmaCNIRu/main.cpp:15:34: warning: Killed: Replaced == with != [cxx_eq_to_ne]
          bool test2 = valid_age(20) == false;
                                     ^
    /tmp/sc-PzmaCNIRu/main.cpp:21:34: warning: Killed: Replaced == with != [cxx_eq_to_ne]
          bool test3 = valid_age(21) == true;
                                     ^
    [info] All mutations have been killed
    [info] Mutation score: 100%
    [info] Total execution time: 554ms

In this last run, we see that all mutants were killed since we covered with tests
all cases around the ``<=``.

Summary
-------

This is a short summary of what we have learned in the tutorial.
Your code has to be compiled with ``-fembed-bitcode -g`` compile flags:

  - Mull expects embedded bitcode files to be present in a binary executable
    (ensured by ``-fembed-bitcode``).

  - Mull needs debug information to be included by the compiler (enabled by
    ``-g``). Mull uses this information to find mutations in bitcode and source
    code.

The next step is to learn about `Compilation Database and Junk Mutations <CompilationDatabaseAndJunk.html>`_
