int math_mul(int a, int b) {
  return a * b;
}

int test_math_mul() {
  if (math_mul(6, 3) == 18) {
    return 0;
  }
  return 1;
}

int main() {
  return test_math_mul();
}

// clang-format off

// RUN: cd / && %clang_cc -fembed-bitcode -g -O0 %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -test-framework CustomTest -mutators=cxx_mul_to_div -ide-reporter-show-killed -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:12: warning: Killed: Replaced * with / [cxx_mul_to_div]
// CHECK:  return a * b;
// CHECK:           ^
// CHECK: [info] All mutations have been killed
