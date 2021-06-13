Keeping mutants under control
=============================

This tutorial shows you how to keep the number of mutants under control.
It builds on top of the `Compilation Database and Junk Mutations <CompilationDatabaseAndJunk.html>`_
tutorial so make sure you go through that one first.

When you apply mutation testing for the first time, you might be overwhelmed by
the number of mutants - what do you do when you see that several hundred or thousands of mutants survived?

The right way to go about it is to put the number of mutants under control and
work through them incrementally.

Mutation Operators
------------------

If you apply Mull with the default set of mutation operators on fmtlib, you will get
around ~4000 mutants, ~3300 of which survive.

.. code-block:: bash

    $ mull-cxx \
        -linker=clang++ \
        -compdb-path compile_commands.json \
        -compilation-flags="\
          -isystem /opt/llvm/10.0.0/include/c++/v1 \
          -isystem /opt/llvm/10.0.0/lib/clang/10.0.0/include \
          -isystem /usr/include \
          -isystem /usr/local/include" \
        ./bin/core-test

    /// skipped

    [info] Survived mutants (3397/3946):
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/core.h:734:19: warning: Survived: Replaced <= with < [cxx_le_to_lt]
        size_ = count <= capacity_ ? count : capacity_;
                      ^
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/core.h:1716:12: warning: Survived: Replaced >= with > [cxx_ge_to_gt]
        if (id >= detail::max_packed_args) return arg;
               ^
    /// skipped

    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format-inl.h:1283:51: warning: Survived: Replaced ++x with --x [cxx_pre_inc_to_pre_dec]
        for (size_t i = 0, n = bigits_.size(); i < n; ++i) {
                                                      ^
    [info] Mutation score: 13%
    [info] Total execution time: 89999ms

Going through all of them to see which ones deserve your attention is
simply impractical.

The easiest way to decrease this number is to pick one or two `mutation operators <SupportedMutations.html>`_.

Let's see how the numbers change if we only use ``cxx_add_to_sub`` that replaces
all the ``a + b`` to ``a - b``.

.. code-block:: bash

    $ mull-cxx \
        -linker=clang++ \
        -mutators=cxx_add_to_sub \
        -compdb-path compile_commands.json \
        -compilation-flags="\
          -isystem /opt/llvm/10.0.0/include/c++/v1 \
          -isystem /opt/llvm/10.0.0/lib/clang/10.0.0/include \
          -isystem /usr/include \
          -isystem /usr/local/include" \
        ./bin/core-test

    /// skipped

    [info] Survived mutants (305/350):
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:1758:34: warning: Survived: Replaced + with - [cxx_add_to_sub]
      state_ = (1103515245U * state_ + 12345U) % kMaxRange;
                                     ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2275:55: warning: Survived: Replaced + with - [cxx_add_to_sub]
      return static_cast<TimeInMillis>(now.tv_sec) * 1000 + now.tv_usec / 1000;
                                                          ^
    /// skipped
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format-inl.h:1334:68: warning: Survived: Replaced + with - [cxx_add_to_sub]
      int num_bigits() const { return static_cast<int>(bigits_.size()) + exp_; }
                                                                       ^
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format-inl.h:1284:53: warning: Survived: Replaced + with - [cxx_add_to_sub]
          double_bigit result = bigits_[i] * wide_value + carry;
                                                        ^
    [info] Mutation score: 12%
    [info] Total execution time: 18481ms

You are still getting plenty - 305 survived out of 350 total, but this is much more
manageable.

Filters
-------

You may notice that the last run had, among others, the following mutants survived:

.. code-block:: text

    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:1758:34: warning: Survived: Replaced + with - [cxx_add_to_sub]
      state_ = (1103515245U * state_ + 12345U) % kMaxRange;
                                     ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2275:55: warning: Survived: Replaced + with - [cxx_add_to_sub]
      return static_cast<TimeInMillis>(now.tv_sec) * 1000 + now.tv_usec / 1000;
                                                          ^

Looking at the paths, it is clear that these mutants are part of the GoogleTest
framework (``gmock-gtest-all.cc``). It is very unlikely that you are interested
in seeing these in the result.
Mull comes with two path-based filters ``--exclude-path`` and ``--include-path``.
You can use these to either exclude or include mutations based on their file-system location.
Let's exclude everything related to GoogleTest:


.. code-block:: bash

    $ mull-cxx \
        -linker=clang++ \
        -mutators=cxx_add_to_sub \
        -exclude-path=".*gtest.*" \
        -exclude-path=".*gmock.*" \
        -compdb-path compile_commands.json \
        -compilation-flags="\
          -isystem /opt/llvm/10.0.0/include/c++/v1 \
          -isystem /opt/llvm/10.0.0/lib/clang/10.0.0/include \
          -isystem /usr/include \
          -isystem /usr/local/include" \
        ./bin/core-test
    /// skipped

    [info] Survived mutants (275/320):
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format-inl.h:228:35: warning: Survived: Replaced + with - [cxx_add_to_sub]
      return i >= 0 ? i * char_digits + count_digits<4, unsigned>(n.value[i]) : 1;
                                      ^

275/320 vs. 305/350. Better, but still too much.

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
