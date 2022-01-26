int math_sub(int a, int b) {
  return a - b;
}

int test_math_sub() {
  if (math_sub(2, 4) == -2) {
    return 0;
  }
  return 1;
}

int main() {
  return test_math_sub();
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:12: warning: Killed: Replaced - with + [cxx_sub_to_add]
// CHECK:  return a - b;
// CHECK:           ^
// CHECK: [info] All mutations have been killed
