fmtlib tutorial
================

This tutorial will show you how to run Mull against `fmtlib <https://github.com/fmtlib/fmt>`_.

Get sources and build fmtlib:

.. code-block:: bash

    git clone https://github.com/fmtlib/fmt.git
    cd fmt
    mkdir build.dir
    cd build.dir
    cmake \
      -DCMAKE_CXX_FLAGS="-fembed-bitcode -g -O0" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
    make core-test

Run Mull against the ``core-test``:

.. code-block:: bash

    mull-cxx -test-framework=GoogleTest -mutators=cxx_add_to_sub ./bin/core-test


If everything works, you will see a number of confusing mutations within the report:

.. code-blocK:: text

    /opt/llvm/9.0.0/bin/../include/c++/v1/__tree:2114:5: warning: Survived: Replaced + with - [cxx_add_to_sub]
        ++size();
        ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:1738:31: warning: Survived: Replaced + with - [cxx_add_to_sub]
        if (specs.width != 0) --specs.width;
                              ^

This is because of ``Junk Mutations``.

Junk Mutations
**************

Not every mutation found at Bitcode level can be represented at the source
level. Mull can filter them out by looking at the source code, but for that you need to
provide `compilation database <https://clang.llvm.org/docs/JSONCompilationDatabase.html>`_,
or compilation flags, or both.

**Please, note:** Clang adds implicit header search paths, which must be added
explicitly via ``-compilation-flags``. You can get them using the following
commands, for C and C++ respectively:

.. code-block:: bash

    > clang -x c -c /dev/null -v
    ... skipped
    #include <...> search starts here:
     /usr/local/include
     /opt/llvm/5.0.0/lib/clang/5.0.0/include
     /usr/include
     /System/Library/Frameworks (framework directory)
     /Library/Frameworks (framework directory)
    End of search list.

.. code-block:: bash

    > clang++ -x c++ -c /dev/null -v
    #include <...> search starts here:
     /opt/llvm/5.0.0/include/c++/v1
     /usr/local/include
     /opt/llvm/5.0.0/lib/clang/5.0.0/include
     /usr/include
     /System/Library/Frameworks (framework directory)
     /Library/Frameworks (framework directory)
    End of search list.

The paths on your machine might be different, but based on the output above you need the following include dirs:

.. code-block:: text

     /opt/llvm/5.0.0/include/c++/v1
     /usr/local/include
     /opt/llvm/5.0.0/lib/clang/5.0.0/include
     /usr/include

Here is how you can run Mull with junk detection enabled:

.. code-block:: bash

    mull-cxx -test-framework=GoogleTest \
      -mutators=cxx_add_to_sub \
      -compdb-path compile_commands.json \
      -compilation-flags="\
        -isystem /opt/llvm/5.0.0/include/c++/v1 \
        -isystem /opt/llvm/5.0.0/lib/clang/5.0.0/include \
        -isystem /usr/include \
        -isystem /usr/local/include" \
        ./bin/core-test

You should see similar output:

.. code-block:: text

    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 154ms
    [info] Loading bitcode files (threads: 4)
           [################################] 4/4. Finished in 305ms
    [info] Compiling instrumented code (threads: 4)
           [################################] 4/4. Finished in 5628ms
    [info] Loading dynamic libraries (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Searching tests (threads: 1)
           [################################] 1/1. Finished in 2ms
    [info] Preparing original test run (threads: 1)
           [################################] 1/1. Finished in 96ms
    [info] Running original tests (threads: 8)
           [################################] 36/36. Finished in 92ms
    [info] Applying function filter: no debug info (threads: 8)
           [################################] 3624/3624. Finished in 13ms
    [info] Applying function filter: file path (threads: 8)
           [################################] 3624/3624. Finished in 11ms
    [info] Instruction selection (threads: 8)
           [################################] 3624/3624. Finished in 27ms
    [info] Searching mutants across functions (threads: 8)
           [################################] 3624/3624. Finished in 11ms
    [info] Applying filter: no debug info (threads: 8)
           [################################] 102/102. Finished in 0ms
    [info] Applying filter: file path (threads: 8)
           [################################] 102/102. Finished in 1ms
    [info] Applying filter: junk (threads: 8)
           [################################] 102/102. Finished in 3397ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Cloning functions for mutation (threads: 4)
           [################################] 4/4. Finished in 45ms
    [info] Removing original functions (threads: 4)
           [################################] 4/4. Finished in 22ms
    [info] Redirect mutated functions (threads: 4)
           [################################] 4/4. Finished in 13ms
    [info] Applying mutations (threads: 1)
           [################################] 41/41. Finished in 13ms
    [info] Compiling original code (threads: 4)
           [################################] 4/4. Finished in 3901ms
    [info] Running mutants (threads: 8)
           [################################] 41/41. Finished in 578ms
    [info] Survived mutants (22/41):
    /tmp/sc-UBe3GUa96/fmt/test/gmock-gtest-all.cc:9758:53: warning: Survived: Replaced + with - [cxx_add_to_sub]
        const int actual_to_skip = stack_frames_to_skip + 1;
                                                        ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:1466:42: warning: Survived: Replaced + with - [cxx_add_to_sub]
        if (negative) abs_value = ~abs_value + 1;
                                             ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:843:53: warning: Survived: Replaced + with - [cxx_add_to_sub]
            (static_cast<uint64_t>(integral) << -one.e) + fractional;
                                                        ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:854:31: warning: Survived: Replaced + with - [cxx_add_to_sub]
            static_cast<char>('0' + static_cast<char>(fractional >> -one.e));
                                  ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:1096:33: warning: Survived: Replaced + with - [cxx_add_to_sub]
            min_exp - (normalized.e + fp::significand_size), cached_exp10);
                                    ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:843:53: warning: Survived: Replaced + with - [cxx_add_to_sub]
            (static_cast<uint64_t>(integral) << -one.e) + fractional;
                                                        ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:844:53: warning: Survived: Replaced + with - [cxx_add_to_sub]
        result = handler.on_digit(static_cast<char>('0' + digit),
                                                        ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:854:31: warning: Survived: Replaced + with - [cxx_add_to_sub]
            static_cast<char>('0' + static_cast<char>(fractional >> -one.e));
                                  ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:1678:30: warning: Survived: Replaced + with - [cxx_add_to_sub]
        auto&& it = reserve(size + padding * fill_size);
                                 ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:415:34: warning: Survived: Replaced + with - [cxx_add_to_sub]
        fp upper = normalize<0>(fp(f + half_ulp, e));
                                     ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:466:69: warning: Survived: Replaced + with - [cxx_add_to_sub]
          ((min_exponent + fp::significand_size - 1) * one_over_log2_10 +
                                                                        ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:946:27: warning: Survived: Replaced + with - [cxx_add_to_sub]
        uint64_t down = (diff + 1) * unit;  // wp_Wdown
                              ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:948:20: warning: Survived: Replaced + with - [cxx_add_to_sub]
            (remainder + divisor < down ||
                       ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:949:39: warning: Survived: Replaced + with - [cxx_add_to_sub]
             down - remainder > remainder + divisor - down)) {
                                          ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:1184:31: warning: Survived: Replaced + with - [cxx_add_to_sub]
        int full_exp = num_digits + exp - 1;
                                  ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:446:67: warning: Survived: Replaced + with - [cxx_add_to_sub]
      return (static_cast<uint64_t>(product) & (1ULL << 63)) != 0 ? f + 1 : f;
                                                                      ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:932:20: warning: Survived: Replaced + with - [cxx_add_to_sub]
            (remainder + divisor < d || d - remainder >= remainder + divisor - d)) {
                       ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format-inl.h:932:64: warning: Survived: Replaced + with - [cxx_add_to_sub]
            (remainder + divisor < d || d - remainder >= remainder + divisor - d)) {
                                                                   ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:886:54: warning: Survived: Replaced + with - [cxx_add_to_sub]
        *--buffer = static_cast<Char>(data::digits[index + 1]);
                                                         ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:1466:42: warning: Survived: Replaced + with - [cxx_add_to_sub]
        if (negative) abs_value = ~abs_value + 1;
                                             ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:886:54: warning: Survived: Replaced + with - [cxx_add_to_sub]
        *--buffer = static_cast<Char>(data::digits[index + 1]);
                                                         ^
    /tmp/sc-UBe3GUa96/fmt/include/fmt/format.h:892:39: warning: Survived: Replaced + with - [cxx_add_to_sub]
        *--buffer = static_cast<Char>('0' + value);
                                          ^
    [info] Mutation score: 46%


Now the mutants are valid and point to the right places.
