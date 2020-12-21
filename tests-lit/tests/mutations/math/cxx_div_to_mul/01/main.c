int math_div(int a, int b) {
  return a / b;
}

int test_math_div() {
  if (math_div(6, 3) == 2) {
    return 0;
  }
  return 1;
}

int main() {
  return test_math_div();
}

// clang-format off

// RUN: cd / && %clang_cc -fembed-bitcode -g -O0 %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -test-framework CustomTest -mutators=cxx_div_to_mul -ide-reporter-show-killed -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:12: warning: Killed: Replaced / with * [cxx_div_to_mul]
// CHECK:  return a / b;
// CHECK:           ^
// CHECK: [info] All mutations have been killed
