int replace_assignment(int a) {
  int b = a + 100;
  return b;
}

int main() {
  return replace_assignment(1) != 101;
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: cd / && %clang_cc %sysroot -fembed-bitcode -g -O0 %s -o %s-ebc.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" -disable-junk-detection -keep-executable -output=%s.mutated.exe -mutators=cxx_assign_const -ide-reporter-show-killed -reporters=IDE %s-ebc.exe | %filecheck %s --dump-input=fail
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s.mutated.exe | %filecheck %s --dump-input=fail
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.c:2:7: warning: Killed: Replaced 'a = b' with 'a = 42' [cxx_assign_const]
// CHECK:  int b = a + 100;
// CHECK:      ^
// CHECK:[info] All mutations have been killed
