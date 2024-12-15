int max(int a, int b) {
  if (a < b) {
    return b;
  } else {
    return a;
  }
}

int test_max() {
  if (max(2, 4) == 4) {
    return 0;
  }
  return 1;
}

int main() {
  return test_max();
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:9: warning: Killed: Replaced < with >= [cxx_lt_to_ge]
// CHECK:  if (a < b) {
// CHECK:        ^
// CHECK: [info] All mutations have been killed
