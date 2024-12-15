void noop(int a, int b) {
  a + b;
}

int main() {
  return 0;
}

// clang-format off
// RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %s.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -include-not-covered %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// RUN: env LLVM_PROFILE_FILE=%s.profraw %s.exe
// RUN: %llvm_profdata merge %s.profraw -o %s.profdata
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -include-not-covered -coverage-info %s.profdata %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Not Covered mutants (1/1):
// CHECK:{{^.*}}main.c:2:5: warning: Not Covered: Replaced + with - [cxx_add_to_sub]
// CHECK:[info] Mutation score: 0%
