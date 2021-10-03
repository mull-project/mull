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

// RUN: cd / && %clang_cc %sysroot -fembed-bitcode -g -O0 %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" -mutators=cxx_sub_to_add -ide-reporter-show-killed -reporters=IDE %s.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:12: warning: Killed: Replaced - with + [cxx_sub_to_add]
// CHECK:  return a - b;
// CHECK:           ^
// CHECK: [info] All mutations have been killed
