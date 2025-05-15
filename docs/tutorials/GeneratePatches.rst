Generate Patches
================

.. include:: ./_support.rst

This tutorial demonstrates how to generate patches for mutants. The patches can be further applied on the original source code to generate mutated version of the codebase.

This tutorial is based on the :doc:`fmtlib example <./CMakeIntegration>`.

Step 1. Check out the source code
---------------------------------

.. code-block:: bash

    git clone https://github.com/fmtlib/fmt --depth 1
    cd fmt


Step 2. Create config ``mull.yml``
----------------------------------

.. code-block:: yaml

    mutators:
      - cxx_add_to_sub
    excludePaths:
      - .*gtest.*
      - .*gmock.*

Step 3. Configure and build fmtlib
----------------------------------

.. code-block:: bash

    mkdir build.dir
    export CXX=clang++-18
    cmake \
      -DCMAKE_CXX_FLAGS="-O0 -fpass-plugin=/usr/lib/mull-ir-frontend-18 -g -grecord-command-line" \
      -S $PWD -B build.dir
    cmake --build build.dir/ -t core-test


Step 4. Run Mull against fmtlib tests in a dry-run mode
-------------------------------------------------------

.. code-block:: bash

    mull-runner-18 --reporters=Patches --report-name fmtlib --report-patch-base $PWD ./build.dir/bin/core-test

You should see similar output:

.. code-block:: bash

    [info] Using config /workspaces/mull/tests/end2end/fmt/mull.yml
    [info] Warm up run (threads: 1)
          [################################] 1/1. Finished in 186ms
    [info] Filter mutants (threads: 1)
          [################################] 1/1. Finished in 0ms
    [info] Baseline run (threads: 1)
          [################################] 1/1. Finished in 182ms
    [info] Running mutants (threads: 8)
          [################################] 42/42. Finished in 1595ms
    [info] Patchfiles can be found at './fmtlib-patches'
    [info] Total execution time: 2002ms
    [info] Surviving mutants: 37

Generated patches are located under ``fmtlib-patches``:

.. code-block:: bash

    $ ls -l fmtlib-patches/
    total 168
    -rw-r--r-- 1 root root 376 May 15 21:47 killed-workspaces_mull_tests_end2end_fmt_include_fmt_format_h-cxx_add_to_sub-L2356-C47.patch
    -rw-r--r-- 1 root root 348 May 15 21:47 killed-workspaces_mull_tests_end2end_fmt_include_fmt_format_h-cxx_add_to_sub-L395-C32.patch
    -rw-r--r-- 1 root root 254 May 15 21:47 killed-workspaces_mull_tests_end2end_fmt_include_fmt_format_h-cxx_add_to_sub-L599-C17.patch
    -rw-r--r-- 1 root root 324 May 15 21:47 killed-workspaces_mull_tests_end2end_fmt_include_fmt_format_h-cxx_add_to_sub-L901-C53.patch
    -rw-r--r-- 1 root root 326 May 15 21:47 killed-workspaces_mull_tests_end2end_fmt_include_fmt_format_h-cxx_add_to_sub-L979-C52.patch
    -rw-r--r-- 1 root root 254 May 15 21:47 survived-workspaces_mull_tests_end2end_fmt_include_fmt_core_h-cxx_add_to_sub-L279-C19.patch
    -rw-r--r-- 1 root root 358 May 15 21:47 survived-workspaces_mull_tests_end2end_fmt_include_fmt_format-inl_h-cxx_add_to_sub-L449-C29.patch
    # ...

The patches can be applied now:

.. code-block:: bash

    $ patch -p1 < ./fmtlib-patches/killed-workspaces_mull_tests_end2end_fmt_include_fmt_format_h-cxx_add_to_sub-L395-C32.patch
    patching file include/fmt/format.h
    $ git diff

.. code-block:: diff

    diff --git a/include/fmt/format.h b/include/fmt/format.h
    index 1bb24a5..946c798 100644
    --- a/include/fmt/format.h
    +++ b/include/fmt/format.h
    @@ -392,7 +392,7 @@ inline T *make_checked(T *p, std::size_t) { return p; }
    template <typename T>
    template <typename U>
    void basic_buffer<T>::append(const U *begin, const U *end) {
    -  std::size_t new_size = size_ + internal::to_unsigned(end - begin);
    +  std::size_t new_size = size_ - internal::to_unsigned(end - begin);
      reserve(new_size);
      std::uninitialized_copy(begin, end,
                              internal::make_checked(ptr_, capacity_) + size_);
