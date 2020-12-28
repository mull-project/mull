int replace_assignment(int a) {
  int b = a + 100;
  return b;
}

int main() {
  return replace_assignment(1) != 101;
}

// clang-format off

// RUN: cd / && %clang_cc -fembed-bitcode -g -O0 %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %MULL_EXEC -test-framework CustomTest -disable-junk-detection -mutators=cxx_assign_const -ide-reporter-show-killed -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:7: warning: Killed: Replaced 'a = b' with 'a = 42' [cxx_assign_const]
// CHECK:  int b = a + 100;
// CHECK:      ^
// CHECK:[info] All mutations have been killed
