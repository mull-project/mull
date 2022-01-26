// clang-format off
/**
RUN: %mull_runner -version 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:Mull: Practical mutation testing for C and C++
CHECK:Home: https://github.com/mull-project/mull
CHECK:Docs: https://mull.readthedocs.io
CHECK:{{^Version: \d+\.\d+.\d+(-pr[0-9]+|-trunk[0-9]+)?$}}
CHECK:{{^Commit: [a-h0-9]+$}}
CHECK:{{^Date: .*$}}
CHECK:{{^LLVM: \d+\.\d+.\d+$}}
CHECK-EMPTY:
*/
