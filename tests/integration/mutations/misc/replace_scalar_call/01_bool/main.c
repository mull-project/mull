#include <stdbool.h>

static bool is_positive(int x) {
  return x > 0;
}

static int check(int x) {
  return is_positive(x);
}

// mull-off
int main(void) {
  return !(check(1) == 1 && check(-1) == 0);
}
// mull-on

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (2/2):
// CHECK-DAG:{{.*}}main.c:8:10: warning: Killed: Replaced with true [cxx_replace_scalar_call_nz]
// CHECK-DAG:{{.*}}main.c:8:10: warning: Killed: Replaced with false [cxx_replace_scalar_call_zr]
// CHECK:[info] All mutations have been killed
