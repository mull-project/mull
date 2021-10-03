int test(int a, int b) {
  return a + b;
}

int main(int argc, char **argv) {
  return !( test(2, 3) == 5 );
}

// clang-format off

// RUN: %clang_cc %sysroot %s -fembed-bitcode -g -o %s.exe

// RUN: cd %S

// RUN: %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" -ide-reporter-show-killed -keep-executable -mutators=cxx_add_to_sub -mutators=cxx_mul_to_div main.c.exe | %filecheck %s --dump-input=fail --match-full-lines --check-prefix=CHECK-MULL-CXX
// CHECK-MULL-CXX: [info] Killed mutants (1/1):

// RUN: %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" -keep-executable -mutate-only -output=main.c.mutated.exe -mutators=cxx_add_to_sub -mutators=cxx_mul_to_div main.c.exe | %filecheck %s --dump-input=fail --match-full-lines --check-prefix=CHECK-MUTATE
// CHECK-MUTATE: [info] Mutate-only mode on:{{.*}}

// RUN: %mull_runner main.c.mutated.exe -ide-reporter-show-killed | %filecheck %s --dump-input=fail --match-full-lines --check-prefix=CHECK-RUNNER
// CHECK-RUNNER: [info] Killed mutants (1/1):
