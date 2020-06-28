OpenSSL tutorial
================

This tutorial will show you how to run Mull against `OpenSSL <https://github.com/openssl/openssl>`_.
This tutorial is similar to the `fmtlib tutorial <fmtlibTutorial.html>`_, but there are two key differences:

- fmtlib uses CMake-based build system, while OpenSSL' build system is very custom and constitutes of a number of shell-scripts
- fmtlib is written in C++, while OpenSSL has some (optional) assembly code
- fmtlib's build system gives us a nice, ready to use compilation database, while for OpenSSL we need to mimic it manually

Let's get started!

Build OpenSSL
*************

Get sources, configure the build system and build everything:

.. code-block::

    git clone https://github.com/openssl/openssl.git
    cd openssl
    export CC=clang
    ./config -no-asm -no-shared -no-module -no-des -no-threads -g -O0 -fembed-bitcode
    make all

Some parts of OpenSSL are written in assembly, but Mull requires LLVM Bitcode to run the program under JIT environment.
To avoid assembly we ask OpenSSL to no use assembly, but instead fall back to identical C implementation.

If we omit ``-no-shared`` and ``-no-module`` flags, then OpenSSL will build ``libcrypto.dylib`` (or ``.so`` on Linux) and
link all the test executables against the dynamic library. In this case, the test executable will only contain LLVM Bitcode
for the tests, but not for the OpenSSL core: the rest of bitcode is in ``libcrypto.dylib``, which can still be accessed by Mull,
but this feature is not implemented yet. By disabling shared library we force the build system to build self-contained executables.

JITting code that uses ``pthreads`` brings some issues that we were not able to debug and fix yet, so we disable threads as well.

The rest of build flags ask OpenSSL to emit debug info, to not run optimizations, and finally to embed LLVM Bitcode into the binary.

*Note: without -no-des the build fails for some awkward reason which we were to lazy to debug and fix.*

Examine OpenSSL
***************

Let's examine ``bio_enc_test`` test suite:

.. code-block:: bash

    mull-cxx -test-framework=CustomTest -mutators=cxx_lshift_assign_to_rshift_assign test/bio_enc_test

If you run it on Linux you should see the following warnings:

.. code-block:: text

    [warning] Could not find dynamic library: libdl.so.2
    [warning] Could not find dynamic library: libc.so.6

In order to run tests (original or mutated) Mull needs to feed the whole program and all its dependencies into the JIT engine.
This includes dynamic libraries. Mull extracts them from the executable. On macOS, in most of the cases, the dynamic
libraries have the full path. However, on Linux there are only names of the libraries, while the full paths normally determined
by the dynamic linker ``ldd``. Mull does not assume any of the paths, so this is responsibility of the user to provide the paths.

macOS example:

.. code-block:: bash

    > otool -L test/bio_enc_test
    test/bio_enc_test:
        /usr/lib/libSystem.B.dylib

Linux example:

.. code-block:: bash

    > ldd test/bio_enc_test
    linux-vdso.so.1 (0x00007fffb01e0000)
    libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f4df0be8000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f4df05d8000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f4df0dec000)

In this example, On Linux, required libraries located under ``/lib/x86_64-linux-gnu``, so we instruct Mull about the paths via ``-ld-search-path`` CLI option:

.. code-block:: bash

    mull-cxx -test-framework=CustomTest -mutators=cxx_lshift_assign_to_rshift_assign -ld-search-path=/lib/x86_64-linux-gnu test/bio_enc_test


Junk Mutations
**************

Now everything works great, but there is one issue: we asked Mull to only apply ``cxx_lshift_assign_to_rshift_assign`` mutations,
which replaces ``<<=`` with ``>>=`` (see the ``-mutators=cxx_lshift_assign_to_rshift_assign`` option), but there are some weird results:

.. code-block:: bash

    > mull-cxx -test-framework=CustomTest -mutators=cxx_lshift_assign_to_rshift_assign -ld-search-path=/lib/x86_64-linux-gnu test/bio_enc_test
    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 268ms
    ...
    skipped
    ...
    [info] Survived mutants (91/128):
    /home/vagrant/openssl/test/testutil/driver.c:335:30: warning: Survived: Replaced <<= with >>= [cxx_lshift_assign_to_rshift_assign]
                if (all_tests[i].subtest && single_iter == -1) {
                                 ^
    /home/vagrant/openssl/test/testutil/driver.c:369:34: warning: Survived: Replaced <<= with >>= [cxx_lshift_assign_to_rshift_assign]
                    if (all_tests[i].subtest) {
                                     ^
    /home/vagrant/openssl/test/testutil/driver.c:74:34: warning: Survived: Replaced <<= with >>= [cxx_lshift_assign_to_rshift_assign]
        all_tests[num_tests].subtest = subtest;
                                     ^
    ...
    skipped
    ...

This is because not all of the mutations available on the bitcode level can be represented on the source code level.
These are called Junk Mutations. In order to solve this problem and get reasonable results we need to instruct

This is because not every mutation found at Bitcode level can be represented at the source
level. Mull can filter them out by looking at the source code, but for that you need to
provide `compilation database <https://clang.llvm.org/docs/JSONCompilationDatabase.html>`_,
or compilation flags, or both.

In case of the custom build system it is not trivial to get the compilation database, so we have to hand-craft the compilation flags ourselves.

**Please, note:** Clang adds implicit header search paths, which must be added
explicitly via ``-compilation-flags``. You can get them using the following
command:

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

The paths on your machine might be different, but based on the output above you need the following include dirs:

.. code-block:: text

    /usr/local/include
    /usr/lib/llvm-6.0/lib/clang/6.0.0/include
    /usr/include/x86_64-linux-gnu
    /usr/include

The final command to run Mull looks like this:

.. code-block:: bash

    > mull-cxx -test-framework=CustomTest \
     -mutators=cxx_lshift_assign_to_rshift_assign \
     -ld-search-path=/lib/x86_64-linux-gnu \
     -compilation-flags="\
       -D_REENTRANT -DMODULESDIR=\"/usr/local/lib/ossl-modules\"
       -I . -I crypto/modes -I crypto/include -I include -I apps/include \
       -I providers/implementations/include -I providers/common/include \
       -isystem /usr/local/include \
       -isystem /usr/lib/llvm-6.0/lib/clang/6.0.0/include \
       -isystem /usr/include \
       -isystem /usr/include/x86_64-linux-gnu" \
     test/bio_enc_test

If everything is correct, then you will see very similar output:

.. code-block:: text

    [info] Extracting bitcode from executable (threads: 1)
           [################################] 1/1. Finished in 277ms
    [info] Loading bitcode files (threads: 2)
           [################################] 690/690. Finished in 734ms
    [info] Compiling instrumented code (threads: 2)
           [################################] 690/690. Finished in 11ms
    [info] Loading dynamic libraries (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Searching tests (threads: 1)
           [################################] 1/1. Finished in 4ms
    [info] Preparing original test run (threads: 1)
           [################################] 1/1. Finished in 179ms
    [info] Running original tests (threads: 1)
           [################################] 1/1. Finished in 613ms
    [info] Applying function filter: no debug info (threads: 2)
           [################################] 833/833. Finished in 11ms
    [info] Applying function filter: file path (threads: 2)
           [################################] 833/833. Finished in 11ms
    [info] Instruction selection (threads: 2)
           [################################] 833/833. Finished in 21ms
    [info] Searching mutants across functions (threads: 2)
           [################################] 833/833. Finished in 11ms
    [info] Applying filter: no debug info (threads: 2)
           [################################] 128/128. Finished in 11ms
    [info] Applying filter: file path (threads: 2)
           [################################] 128/128. Finished in 10ms
    [info] Applying filter: junk (threads: 2)
           [################################] 128/128. Finished in 659ms
    [info] Prepare mutations (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Cloning functions for mutation (threads: 2)
           [################################] 690/690. Finished in 10ms
    [info] Removing original functions (threads: 2)
           [################################] 690/690. Finished in 11ms
    [info] Redirect mutated functions (threads: 2)
           [################################] 690/690. Finished in 0ms
    [info] Applying mutations (threads: 1)
           [################################] 1/1. Finished in 0ms
    [info] Compiling original code (threads: 2)
           [################################] 690/690. Finished in 23ms
    [info] Running mutants (threads: 1)
           [################################] 1/1. Finished in 630ms
    [info] Survived mutants (1/1):
    /home/vagrant/openssl/crypto/sparse_array.c:96:25: warning: Survived: Replaced <<= with >>= [cxx_lshift_assign_to_rshift_assign]
                        idx <<= OPENSSL_SA_BLOCK_BITS;
                            ^
    [info] Mutation score: 0%

