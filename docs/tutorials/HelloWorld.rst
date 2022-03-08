Hello World Example
===================

.. note::
   We would love to hear from you! ❤️

   Please, `report any issues <https://github.com/mull-project/mull/issues>`_ on GitHub, or
   bring your questions to the `#mull channel <https://discord.gg/Hphp7dW>`_ on Discord.

The goal of this tutorial is to demonstrate how to run Mull on minimal C
programs. After reading it you should have a basic understanding of what
arguments Mull needs in order to create mutations in your programs, run the
mutants and generate mutation testing reports.

**TL;DR version**: if you want to run a single copy and paste example, scroll
down to ``Killing mutants again, all killed`` below.

.. note::
   Clang 9 or newer is required!

----

Step 1: Checking version
------------------------

Mull comes in a form of a compiler plugin and therefore tied to specific versions
of Clang and LLVM. As a consequence of that, tools and plugins have a suffix with
the actual Clang/LLVM version.

This tutorial assumes that you are using Clang 12 and that you have
:doc:`installed <../Installation>` Mull on your system and have the ``mull-runner-12``
executable available:

.. code-block:: bash

    $ mull-runner-12 -version
    Mull: LLVM-based mutation testing
    https://github.com/mull-project/mull
    Version: 0.15.0
    Commit: a4be349e
    Date: 18 Jan 2022
    LLVM: 12.0.1

Step 2: Enabling compiler plugin
--------------------------------

Let's create a C++ program:

.. code-block:: c

    int main() {
      return 0;
    }

and compile it:

.. code-block:: bash

    $ clang-12 main.cpp -o hello-world

We can already try using ``mull-runner`` and see what happens:

.. code-block:: text

    $ mull-runner-12 ./hello-world
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 5ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 4ms
    [info] No mutants found. Mutation score: infinitely high
    [info] Total execution time: 10ms

Notice the ``No mutants found`` message! Now, Mull is ready to work with the executable
but there are no mutants: we haven't compiled the program with the compiler plugin that embeds
mutants into our executable.

Let's fix that!
To pass the plugin to Clang, you need to add a few compiler flags.

.. note::
   For Clang 9, 10, and 11 also pass ``-O1``, otherwise the plugin won't be called.

.. note::
   ``-grecord-command-line`` doesn't currently work if you compile several files in one go,
   e.g. ``clang a.c b.c c.c``. In this case, please remove the flag.

.. code-block:: text

    $ clang-12 -fexperimental-new-pass-manager \
      -fpass-plugin=/usr/local/lib/mull-ir-frontend-12 \
      -g -grecord-command-line \
      main.cpp -o hello-world
    [warning] Mull cannot find config (mull.yml). Using some defaults.

Notice the warning: Mull needs a config.
However, in this tutorial we can ignore the warning and rely on the defaults.

You can learn more about the config :doc:`here </../MullConfig>`.

Let's run ``mull-runner`` again:

.. code-block:: text

    $ mull-runner-12 ./hello-world
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 4ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 6ms
    [info] No mutants found. Mutation score: infinitely high
    [info] Total execution time: 12ms

Still no mutants, but this time it is because we don't have any code Mull can mutate.

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

      /// success
      return 0;
    }

We re-compile this new code using the plugin and run the Mull again. This
time we also want to add an additional flag ``-ide-reporter-show-killed`` which
tells Mull to print killed mutations. Normally we are not interested in seeing
killed mutants in console output but in this tutorial we want to be more
verbose.

.. code-block:: text

    $ clang-12 -fexperimental-new-pass-manager \
            -fpass-plugin=/usr/local/lib/mull-ir-frontend-12 \
            -g -grecord-command-line \
            main.cpp -o hello-world
    $ mull-runner-12 -ide-reporter-show-killed hello-world
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 151ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 3ms
    [info] Running mutants (threads: 4)
           [################################] 4/4. Finished in 10ms
    [info] Killed mutants (3/4):
    /tmp/sc-tTV8a84lL/main.cpp:2:11: warning: Killed: Replaced >= with < [cxx_ge_to_lt]
      if (age >= 21) {
              ^
    /tmp/sc-tTV8a84lL/main.cpp:9:30: warning: Killed: Replaced == with != [cxx_eq_to_ne]
      bool test1 = valid_age(25) == true;
                                 ^
    /tmp/sc-tTV8a84lL/main.cpp:15:30: warning: Killed: Replaced == with != [cxx_eq_to_ne]
      bool test2 = valid_age(20) == false;
                                 ^
    [info] Survived mutants (1/4):
    /tmp/sc-tTV8a84lL/main.cpp:2:11: warning: Survived: Replaced >= with > [cxx_ge_to_gt]
      if (age >= 21) {
              ^
    [info] Mutation score: 75%
    [info] Total execution time: 167ms

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

    $ clang-12 -fexperimental-new-pass-manager \
                  -fpass-plugin=/usr/local/lib/mull-ir-frontend-12 \
                  -g -grecord-command-line \
                  main.cpp -o hello-world
    $ mull-runner-12 -ide-reporter-show-killed hello-world
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 469ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 4ms
    [info] Running mutants (threads: 5)
           [################################] 5/5. Finished in 12ms
    [info] Killed mutants (5/5):
    /tmp/sc-tTV8a84lL/main.cpp:2:11: warning: Killed: Replaced >= with > [cxx_ge_to_gt]
      if (age >= 21) {
              ^
    /tmp/sc-tTV8a84lL/main.cpp:2:11: warning: Killed: Replaced >= with < [cxx_ge_to_lt]
      if (age >= 21) {
              ^
    /tmp/sc-tTV8a84lL/main.cpp:9:30: warning: Killed: Replaced == with != [cxx_eq_to_ne]
      bool test1 = valid_age(25) == true;
                                 ^
    /tmp/sc-tTV8a84lL/main.cpp:15:30: warning: Killed: Replaced == with != [cxx_eq_to_ne]
      bool test2 = valid_age(20) == false;
                                 ^
    /tmp/sc-tTV8a84lL/main.cpp:21:30: warning: Killed: Replaced == with != [cxx_eq_to_ne]
      bool test3 = valid_age(21) == true;
                                 ^
    [info] All mutations have been killed
    [info] Mutation score: 100%
    [info] Total execution time: 487ms

In this last run, we see that all mutants were killed since we covered with tests
all cases around the ``<=``.

Summary
-------

As a summary, all you need to enable Mull is to add a few compiler flags to the
build system and then run ``mull-runner`` against the resulting executable.
Just to recap:

.. code-block:: text

    $ clang-12 -fexperimental-new-pass-manager \
                  -fpass-plugin=/usr/local/lib/mull-ir-frontend-12 \
                  -g -grecord-command-line \
                  main.cpp -o hello-world
    $ mull-runner-12 hello-world

Next Steps
----------

Take a look at :doc:`makefile <./MakefileIntegration>` or :doc:`CMake <./CMakeIntegration>` integrations.
