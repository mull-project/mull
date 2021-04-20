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

// RUN: cd / && %clang_cc -fembed-bitcode -g -O0 %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %MULL_EXEC -keep-executable -output=%s.mutated.exe -linker=%clang_cc -mutators=cxx_lt_to_ge -ide-reporter-show-killed -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s.mutated.exe | %FILECHECK_EXEC %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:9: warning: Killed: Replaced < with >= [cxx_lt_to_ge]
// CHECK:  if (a < b) {
// CHECK:        ^
// CHECK: [info] All mutations have been killed
