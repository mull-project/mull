#include <stdbool.h>

static bool source(void) {
  return false;
}

int main(void) {
  bool flag;
  flag = source();
  return flag;
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:{{[0-9]+}}:{{[0-9]+}}: warning: Killed: Replaced with = true; [cxx_assign_const]
// CHECK:[info] All mutations have been killed
