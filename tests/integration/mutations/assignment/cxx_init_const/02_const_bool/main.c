#include <stdbool.h>

int main(void) {
  bool flag = false;
  return flag;
}

// clang-format off

// The old "replace with 42" produced an equivalent mutant here: bool reads keep
// only the lowest bit of the i8 storage, and 42 is even, so the mutant read back
// as false. The flipped constant (1) is observable.

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:{{[0-9]+}}:{{[0-9]+}}: warning: Killed: Replaced with 1 [cxx_init_const]
// CHECK:[info] All mutations have been killed
