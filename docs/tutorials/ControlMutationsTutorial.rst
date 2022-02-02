Keeping mutants under control
=============================

This tutorial shows how to control the amount of mutatns.

It builds on top of the :doc:`Makefile Integration: OpenSSL <./MakefileIntegration>` and :doc:`CMake Integration: libfmt <./CMakeIntegration>` tutorials.

---

When you apply mutation testing for the first time, you might be overwhelmed by
the number of mutants - what do you do when you see that several hundred or thousands of mutants survived?

The right way to go about it is to put the number of mutants under control and
work through them incrementally.

*Scroll down past the ``OpenSSL`` part if you are coming from the ``CMake tutorial``.*

OpenSSL Example
---------------

-------
Filters
-------

In the last run of the OpenSSL tutorial, you may notice that, among the others, the following mutants survived:

.. code-block:: text

    /private/tmp/sc-yOzi9P1sJ/openssl/test/testutil/format_output.c:301:32: warning: Survived: Replaced + with - [cxx_add_to_sub]
        len = ((l1 > l2 ? l1 : l2) + bytes - 1) / bytes * bytes;
                                   ^
    /private/tmp/sc-yOzi9P1sJ/openssl/test/testutil/random.c:24:54: warning: Survived: Replaced + with - [cxx_add_to_sub]
        test_random_state[pos] += test_random_state[(pos + 28) % 31];
                                                         ^

Looking at the paths, it is clear that these mutants are part of the test framework
framework (``test/testutil/random.c``, ``test/testutil/format_output.c``).
It is very unlikely that you are interested in seeing these in the result.

Mull comes with two path-based filters ``excludePaths`` and ``includePaths``.
You can use these to either exclude or include mutations based on their file-system location.
Let's exclude everything related to the ``testutil``.

Your ``mull.yml`` should look like this:

.. code-block:: yaml

    mutators:
      - cxx_add_to_sub
    excludePaths:
      - .*testutil.*

Now, rebuild OpenSSL:

.. code-block:: bash

    make clean
    make build_generated -j
    make ./test/bio_enc_test -j

And rerun Mull:

.. code-block:: bash

    mull-runner-12 ./test/bio_enc_test


Code Coverage
-------------

In fact, many of the survived mutants can never be detected by the test suite because
they are not reachable by any of the tests. We can leverage code coverage information
to cut off all those mutants.

For that to work, we need to gather the coverage info first.

.. code-block:: bash

    $ cmake \
        -DCMAKE_CXX_FLAGS="-fembed-bitcode -g -O0 -fprofile-instr-generate -fcoverage-mapping" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
    $ make core-test
    $ ./bin/core-test

Running ``core-test`` with the coverage info enabled (``-fprofile-instr-generate -fcoverage-mapping``)
generates raw coverage info in the current folder. Currently, Mull doesn't work with raw info,
so we need to post-process it manually:

.. code-block:: bash

    $ llvm-profdata merge default.profraw -o default.profdata

Now we can pass ``default.profdata`` to Mull. Another important detail, now we
also need to tell Mull about additional linker flags - otherwise, it won't be able
to reconstruct mutated executable. See the ``--linker-flags`` CLI option:

.. code-block:: bash

    $ mull-cxx \
        -linker=clang++ \
        -linker-flags="-fprofile-instr-generate -fcoverage-mapping" \
        -mutators=cxx_add_to_sub \
        -exclude-path=".*gtest.*" \
        -exclude-path=".*gmock.*" \
        -coverage-info=default.profdata \
        -compdb-path compile_commands.json \
        -compilation-flags="\
          -isystem /opt/llvm/10.0.0/include/c++/v1 \
          -isystem /opt/llvm/10.0.0/lib/clang/10.0.0/include \
          -isystem /usr/include \
          -isystem /usr/local/include" \
        ./bin/core-test
    /// skipped

    [info] Survived mutants (14/27):
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format-inl.h:2129:37: warning: Survived: Replaced + with - [cxx_add_to_sub]
      const int beta_minus_1 = exponent + floor_log2_pow10(-minus_k);
                                        ^
    /// skipped
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format.h:1570:31: warning: Survived: Replaced + with - [cxx_add_to_sub]
      auto it = reserve(out, size + padding * specs.fill.size());
                                  ^
    [info] Mutation score: 48%
    [info] Total execution time: 14124ms


Now, we've got only 27 mutants instead of 4000 in the beginning - something we can work with.
It's always a good idea to start with the code coverage in the first place.
In this case, even without filters and changing the set of mutation operators, we can decrease the number of mutants to something much more actionable.

As an exercise, try to remove ``-exclude-path`` and ``-mutators`` options and see
how many mutants you get.

(*Spoiler alert: 563*)
