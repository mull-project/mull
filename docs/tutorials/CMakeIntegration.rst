CMake Integration: fmtlib
=========================

This tutorial demonstrates how to integrate Mull into a CMake-based project.

We use `fmtlib <https://github.com/fmtlib/fmt/blob/master/LICENSE.rst>`_ as an example.

.. note::
   If you are new to Mull, then :doc:`Hello World example <./HelloWorld>` is a good starting point.

Step 1. Check out the source code
---------------------------------

.. code-block:: bash

    git clone https://github.com/fmtlib/fmt --depth 1

Step 2. Create sample Mull config
---------------------------------

Create a file ``fmt/mull.yml`` with the following contents:

.. code-block:: yaml

    mutators:
      - cxx_add_to_sub


Step 3. Configure and build fmtlib
----------------------------------

.. code-block:: bash

    mkdir fmt/build.dir
    cd fmt/build.dir
    export CXX=clang++-12
    cmake \
      -DCMAKE_CXX_FLAGS="-O0 -fexperimental-new-pass-manager -fpass-plugin=/usr/lib/mull-ir-frontend-12 -g -grecord-command-line" \
      ..
    make core-test -j


Step 4. Run Mull against fmtlib tests
-------------------------------------

.. code-block:: bash

    mull-runner-12 ./bin/core-test

You should see similar output:

.. code-block:: bash

    [info] Using config /tmp/sc-ySbkbNvt3/fmt/mull.yml
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Filter mutants (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 8ms
    [info] Running mutants (threads: 8)
           [################################] 164/164. Finished in 363ms
    [info] Survived mutants (160/164):
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/core.h:2237:14: warning: Survived: Replaced + with - [cxx_add_to_sub]
      return len + !len;
                 ^
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/core.h:2267:24: warning: Survived: Replaced + with - [cxx_add_to_sub]
        value = value * 10 + unsigned(*p - '0');
                           ^
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/core.h:2277:31: warning: Survived: Replaced + with - [cxx_add_to_sub]
                     prev * 10ull + unsigned(p[-1] - '0') <= max
                                  ^
    <truncated>
    /tmp/sc-ySbkbNvt3/fmt/test/gtest/gmock-gtest-all.cc:12210:36: warning: Survived: Replaced + with - [cxx_add_to_sub]
                   IsUTF8TrailByte(s[i + 1]) &&
                                       ^
    /tmp/sc-ySbkbNvt3/fmt/test/gtest/gmock-gtest-all.cc:12211:36: warning: Survived: Replaced + with - [cxx_add_to_sub]
                   IsUTF8TrailByte(s[i + 2]) &&
                                       ^
    /tmp/sc-ySbkbNvt3/fmt/test/gtest/gmock-gtest-all.cc:14386:26: warning: Survived: Replaced + with - [cxx_add_to_sub]
            argv[j] = argv[j + 1];
                             ^
    [info] Mutation score: 2%
    [info] Total execution time: 491ms

We've got lots of survived mutants.

We can ignore some of them (specifically the ones coming from ``gtest`` and ``gmock``) by extending the config file as follows:

.. code-block:: yaml

    mutators:
      - cxx_add_to_sub
    excludePaths:
      - .*gtest.*
      - .*gmock.*

After rerunning Mull:

.. code-block:: bash

    $ mull-runner-12 ./bin/core-test
    [info] Using config /tmp/sc-ySbkbNvt3/fmt/mull.yml
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Filter mutants (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Running mutants (threads: 8)
           [################################] 96/96. Finished in 312ms
    [info] Survived mutants (92/96):
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/core.h:2237:14: warning: Survived: Replaced + with - [cxx_add_to_sub]
      return len + !len;
                 ^
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/core.h:2267:24: warning: Survived: Replaced + with - [cxx_add_to_sub]
        value = value * 10 + unsigned(*p - '0');
                           ^
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/core.h:2277:31: warning: Survived: Replaced + with - [cxx_add_to_sub]
                     prev * 10ull + unsigned(p[-1] - '0') <= max
                                  ^
    <truncated>
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/format.h:2335:40: warning: Survived: Replaced + with - [cxx_add_to_sub]
      if (negative) abs_value = ~abs_value + 1;
                                           ^
    /tmp/sc-ySbkbNvt3/fmt/include/fmt/format.h:2337:34: warning: Survived: Replaced + with - [cxx_add_to_sub]
      auto size = (negative ? 1 : 0) + static_cast<size_t>(num_digits);
                                     ^
    [info] Mutation score: 4%
    [info] Total execution time: 368ms

We get fewer mutants, but the number can be reduced even further.

Step 5. Filter out unreachable mutants
--------------------------------------

This step left as an exercise for the reader: :doc:`Keeping mutants under control <./ControlMutationsTutorial>`.

**Hint:** use Code Coverage filter.
