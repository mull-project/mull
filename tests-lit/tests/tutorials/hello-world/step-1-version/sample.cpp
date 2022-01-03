// clang-format off
/**
RUN: %mull_cxx -version 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
RUN: %mull_runner -version 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:Mull: LLVM-based mutation testing
CHECK:https://github.com/mull-project/mull
CHECK:{{^Version: \d+\.\d+.\d+(-pr[0-9]+|-trunk[0-9]+)?$}}
CHECK:{{^Commit: [a-h0-9]+$}}
CHECK:{{^Date: .*$}}
CHECK:{{^LLVM: \d+\.\d+.\d+$}}
CHECK-EMPTY:
*/
