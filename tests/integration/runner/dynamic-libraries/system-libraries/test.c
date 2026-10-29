#include <math.h>

int sum(int a, int b) {
  return a + b;
}

int main() {
  int r = sum(2, 5) == 7 && sqrt(4.0) == 2.0;
  return !r;
}

// clang-format off

/*
System libraries must be resolved without -ld-search-path (https://github.com/mull-project/mull/issues/1024)

RUN: rm -rf %t && mkdir -p %t
RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -grecord-command-line -lm %libcxx -o %t/test-ir.exe

RUN: cd /; unset TERM; %mull_runner -ide-reporter-show-killed %t/test-ir.exe 2>&1 | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not="Could not find dynamic library"
CHECK:{{.*}}test.c:4:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
CHECK:  return a + b;
CHECK:           ^
*/
