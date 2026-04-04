// clang-format off
/**
RUN: %mull_runner -version 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:{{^mull-runner {STABLE_MULL_VERSION}$}}
CHECK:{{.*}}Mull: Practical mutation testing and fault injection for C and C++
CHECK:Home: https://mull-project.com
CHECK:{{^LLVM: \d+\.\d+.\d+$}}
CHECK-EMPTY:
*/
