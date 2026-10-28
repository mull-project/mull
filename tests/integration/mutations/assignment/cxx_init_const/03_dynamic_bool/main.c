#include <stdbool.h>

static bool source(void) {
  return false;
}

int main(void) {
  bool flag = source();
  return flag;
}

// clang-format off

// A dynamic bool store (`store i8 (zext i1 ...)`) cannot be flipped with a
// constant, so it is replaced with true; the old 42 always read back as false.

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:{{[0-9]+}}:{{[0-9]+}}: warning: Killed: Replaced with true [cxx_init_const]
// CHECK:[info] All mutations have been killed
