#include "generated.inc"

int kept(int a, int b) {
  return a + b;
}

int main() {
  return skipped(1, 2) == 3 && kept(2, 5) == 7 ? 0 : 1;
}

// clang-format off

/**
Mutant identifiers contain the file path, which has to be UTF-8 for mull-runner to read it. macOS does
not allow such file names, so generated.inc refers to one with a #line directive.

RUN: cd %S && printf '# 1 "caf\351.c"\nint skipped(int a, int b) {\n  return a + b;\n}\n' > generated.inc
RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %pass_mull_ir_frontend -g -grecord-command-line %s -o %s.exe 2>&1 | %filecheck %s --dump-input=fail --check-prefix=CHECK-MUTATE
RUN: cd %S && unset TERM; env MULL_CONFIG=%S/mull.yml %mull_runner --ide-reporter-show-killed %s.exe 2>&1 | %filecheck %s --dump-input=fail

CHECK-MUTATE:[warning] Skipping a mutant in a non-UTF-8 path: {{.*}}

CHECK:[info] Killed mutants (1/1):
CHECK:{{.*}}main.c:4:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
**/
