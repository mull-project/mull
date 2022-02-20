Compilation Database and Junk Mutations
=======================================

.. warning::
   Since the version 0.15.0, ``mull-cxx`` tool `is deprecated <https://github.com/mull-project/mull/issues/945>`_
   in favour of a compiler plugin: `Mull IR Frontend <https://github.com/mull-project/mull/pull/938>`_.

   This tutorial covers ``mull-cxx`` tool and will be removed in the future.

   It is recommended that you use the new :doc:`Mull IR Frontend <./HelloWorld>`.

This tutorial shows how to apply Mull on a real-world project and
how to overcome typical issues you might face.

This tutorial uses `fmtlib <https://github.com/fmtlib/fmt>`_ as an example.

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

*Note: last tested against commit 018688da2a58ba25cdf173bd899734f755adb11a*

Run Mull against the ``core-test``:

.. code-block:: bash

    mull-cxx -mutators=cxx_add_to_sub ./bin/core-test


Right now you should see a weird and long error message by the end of execution.
Here is a snippet:

.. code-block:: text

    /// skipped
    [info] Applying mutations (threads: 1)
           [################################] 10/10. Finished in 11ms
    [info] Compiling original code (threads: 4)
           [################################] 4/4. Finished in 5671ms
    [info] Link mutated program (threads: 1)
    [error] Cannot link program
    status: Failed
    time: 20096ms
    exit: -60
    command: clang /var/folders/1s/hps840w156xfn_m__h2m17880000gp/T/mull-b5963a.o /var/folders/1s/hps840w156xfn_m__h2m17880000gp/T/mull-62252e.o /var/folders/1s/hps840w156xfn_m__h2m17880000gp/T/mull-22ed08.o /var/folders/1s/hps840w156xfn_m__h2m17880000gp/T/mull-84dd4a.o -o /var/folders/1s/hps840w156xfn_m__h2m17880000gp/T/mull-84a88a.exe
    stdout:
    stderr: Undefined symbols for architecture x86_64:
      "std::logic_error::what() const", referenced from:
          vtable for assertion_failure in mull-b5963a.o
      "std::runtime_error::what() const", referenced from:
          vtable for testing::internal::GoogleTestFailureException in mull-22ed08.o
          vtable for fmt::v7::format_error in mull-84dd4a.o
          vtable for fmt::v7::system_error in mull-84dd4a.o
    /// skipped

In order to do the job, Mull takes the executable, deconstructs it into a number
of pieces, inserts mutations into those pieces, and then constructs the executable
again. In order to re-build the mutated program, Mull needs a linker. As a safe default,
it just uses ``clang`` which works in most of the cases. However, in this case
we deal with C++ which needs a corresponding C++ linker. Instead we should be
using ``clang++``, which will do the job just fine.

*Note: on Linux you may have to specify clang-<version> or clang++-<version>,
where <version> corresponds to the version of clang installed*

Try this:

.. code-block:: bash

    mull-cxx --linker=clang++ -mutators=cxx_add_to_sub ./bin/core-test

You should see similar output:

.. code-block:: text

    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 194ms
    [info] Loading bitcode files (threads: 4)
           [################################] 4/4. Finished in 484ms
    [info] Sanity check run (threads: 1)
           [################################] 1/1. Finished in 12ms
    [info] Gathering functions under test (threads: 1)
           [################################] 1/1. Finished in 5ms
    [info] Applying function filter: no debug info (threads: 8)
           [################################] 12812/12812. Finished in 26ms
    [info] Applying function filter: file path (threads: 8)
           [################################] 12418/12418. Finished in 68ms
    [info] Instruction selection (threads: 8)
           [################################] 12418/12418. Finished in 291ms
    [info] Searching mutants across functions (threads: 8)
           [################################] 12418/12418. Finished in 42ms
    [info] Applying filter: no debug info (threads: 8)
           [################################] 863/863. Finished in 1ms
    [info] Applying filter: file path (threads: 8)
           [################################] 863/863. Finished in 11ms
    [info] Applying filter: junk (threads: 8)
    /tmp/sc-0Puh0WBoL/fmt/test/./gmock-gtest-all.cc:39:10: fatal error: 'gtest.h' file not found
    #include "gtest.h"
             ^~~~~~~~~
    [warning] Cannot parse file: '/tmp/sc-0Puh0WBoL/fmt/test/./gmock-gtest-all.cc':
    mull-cxx /tmp/sc-0Puh0WBoL/fmt/test/./gmock-gtest-all.cc
    Make sure that the flags provided to Mull are the same flags that are used for normal compilation.
    /tmp/sc-0Puh0WBoL/fmt/test/./core-test.cc:8:10: fatal error: 'algorithm' file not found
    #include <algorithm>
             ^~~~~~~~~~~
           [--------------------------------] 1/863
    [warning] Cannot parse file: '/tmp/sc-0Puh0WBoL/fmt/test/./core-test.cc':
    mull-cxx /tmp/sc-0Puh0WBoL/fmt/test/./core-test.cc
    Make sure that the flags provided to Mull are the same flags that are used for normal compilation.
    /tmp/sc-0Puh0WBoL/fmt/src/format.cc:8:10: fatal error: 'fmt/format-inl.h' file not found
    #include "fmt/format-inl.h"
             ^~~~~~~~~~~~~~~~~~
    [warning] Cannot parse file: '/tmp/sc-0Puh0WBoL/fmt/src/format.cc':
    mull-cxx /tmp/sc-0Puh0WBoL/fmt/src/format.cc
    Make sure that the flags provided to Mull are the same flags that are used for normal compilation.
           [################################] 863/863. Finished in 160ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Cloning functions for mutation (threads: 4)
           [################################] 4/4. Finished in 51ms
    [info] Removing original functions (threads: 4)
           [################################] 4/4. Finished in 43ms
    [info] Redirect mutated functions (threads: 4)
           [################################] 4/4. Finished in 12ms
    [info] Applying mutations (threads: 1)
           [################################] 10/10. Finished in 10ms
    [info] Compiling original code (threads: 4)
           [################################] 4/4. Finished in 5623ms
    [info] Link mutated program (threads: 1)
           [################################] 1/1. Finished in 402ms
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 597ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 30ms
    [info] Running mutants (threads: 8)
           [################################] 10/10. Finished in 157ms
    [info] Survived mutants (10/10):
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2922:18: warning: Survived: Replaced + with - [cxx_add_to_sub]
        } else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1])) {
                     ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2922:72: warning: Survived: Replaced + with - [cxx_add_to_sub]
        } else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1])) {
                                                                           ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:554:67: warning: Survived: Replaced + with - [cxx_add_to_sub]
                           static_cast<unsigned int>(kMaxRandomSeed)) +
                                                                      ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:566:30: warning: Survived: Replaced + with - [cxx_add_to_sub]
      const int next_seed = seed + 1;
                                 ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:734:37: warning: Survived: Replaced + with - [cxx_add_to_sub]
        const int last_in_range = begin + range_width - 1;
                                        ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:6283:26: warning: Survived: Replaced + with - [cxx_add_to_sub]
            argv[j] = argv[j + 1];
                             ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:6283:26: warning: Survived: Replaced + with - [cxx_add_to_sub]
            argv[j] = argv[j + 1];
                             ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:9763:53: warning: Survived: Replaced + with - [cxx_add_to_sub]
        const int actual_to_skip = stack_frames_to_skip + 1;
                                                        ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:11208:26: warning: Survived: Replaced + with - [cxx_add_to_sub]
            argv[j] = argv[j + 1];
                             ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:11208:26: warning: Survived: Replaced + with - [cxx_add_to_sub]
            argv[j] = argv[j + 1];
                             ^
    [info] Mutation score: 0%
    [info] Total execution time: 8252ms

Almost everything works fine, except of those weird warnings:

.. code-block:: text

    /tmp/sc-0Puh0WBoL/fmt/src/format.cc:8:10: fatal error: 'fmt/format-inl.h' file not found
    #include "fmt/format-inl.h"
             ^~~~~~~~~~~~~~~~~~
    [warning] Cannot parse file: '/tmp/sc-0Puh0WBoL/fmt/src/format.cc':
    mull-cxx /tmp/sc-0Puh0WBoL/fmt/src/format.cc
    Make sure that the flags provided to Mull are the same flags that are used for normal compilation.

That is because of junk mutations.

Junk Mutations
**************

Not every mutation found at Bitcode level can be represented at the source
level. A mutation is called *junk mutation* if it exists on the bitcode level, but
cannot be achieved on the source code level. Mull filters them out by looking back at
the source code. It tries its best, but sometimes it cannot parse the file because it
doesn't have enough information. To give all the information needed you should
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
     /opt/llvm/10.0.0/lib/clang/10.0.0/include
     /System/Library/Frameworks (framework directory)
     /Library/Frameworks (framework directory)
    End of search list.

.. code-block:: bash

    > clang++ -x c++ -c /dev/null -v
    #include <...> search starts here:
     /opt/llvm/10.0.0/bin/../include/c++/v1
     /usr/local/include
     /opt/llvm/10.0.0/lib/clang/10.0.0/include
     /System/Library/Frameworks (framework directory)
     /Library/Frameworks (framework directory)
    End of search list.

The paths on your machine might be different, but based on the output above you need the following include dirs:

.. code-block:: text

     /opt/llvm/10.0.0/include/c++/v1
     /usr/local/include
     /opt/llvm/10.0.0/lib/clang/10.0.0/include
     /usr/include

Here is how you can run Mull with junk detection enabled:

.. code-block:: bash

    mull-cxx \
      -linker=clang++ \
      -mutators=cxx_add_to_sub \
      -compdb-path compile_commands.json \
      -compilation-flags="\
        -isystem /opt/llvm/10.0.0/include/c++/v1 \
        -isystem /opt/llvm/10.0.0/lib/clang/10.0.0/include \
        -isystem /usr/include \
        -isystem /usr/local/include" \
        ./bin/core-test

You should see similar output:

.. code-block:: text

    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 182ms
    [info] Loading bitcode files (threads: 4)
           [################################] 4/4. Finished in 409ms
    [info] Sanity check run (threads: 1)
           [################################] 1/1. Finished in 11ms
    [info] Gathering functions under test (threads: 1)
           [################################] 1/1. Finished in 5ms
    [info] Applying function filter: no debug info (threads: 8)
           [################################] 12812/12812. Finished in 22ms
    [info] Applying function filter: file path (threads: 8)
           [################################] 12418/12418. Finished in 71ms
    [info] Instruction selection (threads: 8)
           [################################] 12418/12418. Finished in 270ms
    [info] Searching mutants across functions (threads: 8)
           [################################] 12418/12418. Finished in 43ms
    [info] Applying filter: no debug info (threads: 8)
           [################################] 863/863. Finished in 12ms
    [info] Applying filter: file path (threads: 8)
           [################################] 863/863. Finished in 10ms
    [info] Applying filter: junk (threads: 8)
           [################################] 863/863. Finished in 4531ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 1ms
    [info] Cloning functions for mutation (threads: 4)
           [################################] 4/4. Finished in 439ms
    [info] Removing original functions (threads: 4)
           [################################] 4/4. Finished in 241ms
    [info] Redirect mutated functions (threads: 4)
           [################################] 4/4. Finished in 12ms
    [info] Applying mutations (threads: 1)
           [################################] 350/350. Finished in 11ms
    [info] Compiling original code (threads: 4)
           [################################] 4/4. Finished in 4570ms
    [info] Link mutated program (threads: 1)
           [################################] 1/1. Finished in 292ms
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 614ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 30ms
    [info] Running mutants (threads: 8)
           [################################] 350/350. Finished in 4421ms
    [info] Survived mutants (305/350):
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:1758:34: warning: Survived: Replaced + with - [cxx_add_to_sub]
      state_ = (1103515245U * state_ + 12345U) % kMaxRange;
                                     ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2275:55: warning: Survived: Replaced + with - [cxx_add_to_sub]
      return static_cast<TimeInMillis>(now.tv_sec) * 1000 + now.tv_usec / 1000;
                                                          ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2922:18: warning: Survived: Replaced + with - [cxx_add_to_sub]
        } else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1])) {
                     ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2922:72: warning: Survived: Replaced + with - [cxx_add_to_sub]
        } else if (i + 1 < num_chars && IsUtf16SurrogatePair(str[i], str[i + 1])) {
                                                                           ^
    /tmp/sc-0Puh0WBoL/fmt/test/gmock-gtest-all.cc:2924:63: warning: Survived: Replaced + with - [cxx_add_to_sub]
              CreateCodePointFromUtf16SurrogatePair(str[i], str[i + 1]);
                                                                  ^

    /// skipped

    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format-inl.h:1334:68: warning: Survived: Replaced + with - [cxx_add_to_sub]
      int num_bigits() const { return static_cast<int>(bigits_.size()) + exp_; }
                                                                       ^
    /tmp/sc-0Puh0WBoL/fmt/include/fmt/format-inl.h:1284:53: warning: Survived: Replaced + with - [cxx_add_to_sub]
          double_bigit result = bigits_[i] * wide_value + carry;
                                                        ^
    [info] Mutation score: 12%
    [info] Total execution time: 16280ms


In the end, 305 out of 350 mutants survived. Why so? One of the reasons is
because most of the mutants are unreachable by the test suite.
You can learn how to handle this issue in the next tutorial: `Keeping mutants under control <ControlMutationsTutorial.html>`_
