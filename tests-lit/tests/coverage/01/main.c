void noop(int a, int b) {
  a + b;
}

int main() {
  return 0;
}

// clang-format off
// TODO: move coverage to runner
// XFAIL: *
// RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %s-ir.exe
// RUN: env LLVM_PROFILE_FILE=%s.profraw %s-ir.exe
// RUN: %llvm_profdata merge %s.profraw -o %s.profdata
// RUN: unset TERM; %mull_runner %s-ir.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NO-COVERAGE
// CHECK-NO-COVERAGE:[info] Survived mutants (1/1):
// CHECK-NO-COVERAGE:{{^.*}}main.c:2:5: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:[info] Mutation score: 0%

// RUN: env MULL_CONFIG=%S/mull.cov.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %s-ir-cov.exe
// RUN: unset TERM; %mull_runner %s-ir-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// CHECK-COVERAGE:[info] No mutants found. Mutation score: infinitely high
