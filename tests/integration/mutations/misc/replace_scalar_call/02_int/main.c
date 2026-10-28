static int twice(int x) {
  return 2 * x;
}

static int apply(int x) {
  return twice(x);
}

// mull-off
int main(void) {
  return apply(3) != 6;
}
// mull-on

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (2/2):
// CHECK-DAG:{{.*}}main.c:6:10: warning: Killed: Replaced with 42 [cxx_replace_scalar_call_nz]
// CHECK-DAG:{{.*}}main.c:6:10: warning: Killed: Replaced with 0 [cxx_replace_scalar_call_zr]
// CHECK:[info] All mutations have been killed
