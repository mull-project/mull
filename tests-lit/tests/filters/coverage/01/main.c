void noop(int a, int b) {
  a + b;
}

int main(int argc, char **argv) {
  if (argc != 1) {
    return 1;
  }
  return 0;
}

// clang-format off
// RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -o %s.exe
// RUN: unset TERM; %mull_runner %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NO-COVERAGE
// CHECK-NO-COVERAGE:[info] Survived mutants (1/1):
// CHECK-NO-COVERAGE:{{^.*}}main.c:2:5: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:[info] Mutation score: 0%

// RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %s-cov.exe
// RUN: unset TERM; %mull_runner %s-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE

// RUN: env LLVM_PROFILE_FILE=%s.profraw %s-cov.exe
// RUN: %llvm_profdata merge %s.profraw -o %s.profdata
// RUN: unset TERM; %mull_runner -coverage-info=%s.profdata %s-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// CHECK-COVERAGE:[info] No mutants found. Mutation score: infinitely high
