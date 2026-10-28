int main(void) {
  int val = 42;
  return val != 42;
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:{{[0-9]+}}:{{[0-9]+}}: warning: Killed: Replaced with 43 [cxx_init_const]
// CHECK:[info] All mutations have been killed
