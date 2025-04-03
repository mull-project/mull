Disable mutants with annotations
================================

.. include:: ./_support.rst

This is a short tutorial showing how to disable mutants using source code annotations.

---

There are two ways to disable/ignore a mutant using source code annotations: for single lines and for bigger blocks of code.

1. Turn Mull off for a big chunk of code:

Any mutations within the block marked by ``mull-off``/``mull-on`` will be ignored.

.. code-block:: cpp

    // disable all mutations from mull-off till mull-on
    // mull-off
    int ignore_all(int a, int b) {
       return a + b;
    }
    // mull-on

    // disable cxx_add_to_sub from mull-off till mull-on
    // mull-off: cxx_add_to_sub
    int ignore_some(int a, int b, int c) {
       return a + b - c;
    }
    // mull-on

    // disable all mutations till the end of file
    // mull-off
    int ignore_till_eof(int a, int b, int c) {
       return a + b - c;
    }

2. Disable mutations for a single line:

.. code-block:: cpp

    int ignore_all_line(int a, int b) {
       // disable all mutations on the annotated line
       return a + b; // mull-ignore
    }

    int ignore_some_line(int a, int b, int c) {
       // disable cxx_add_to_sub on the annotated line
       return a + b - c; // mull-ignore: cxx_add_to_sub
    }

    int ignore_all_next_line(int a, int b) {
       // disable all mutations on the line following the annotation
       // mull-ignore-next
       return a + b;
    }

    int ignore_some_next_line(int a, int b, int c) {
       // disable cxx_add_to_sub on the line following the annotation
       // mull-ignore-next: cxx_add_to_sub
       return a + b - c;
    }
