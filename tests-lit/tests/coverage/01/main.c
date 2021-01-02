void noop(int a, int b) {
  a + b;
}

int main() {
  return 0;
}

// clang-format off

// RUN: cd / && %clang_cc %s -fembed-bitcode -g -fprofile-instr-generate -fcoverage-mapping -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: env LLVM_PROFILE_FILE=%s.profraw %s.exe
// RUN: %llvm_profdata merge %s.profraw -o %s.profdata

// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -linker-flags="-fprofile-instr-generate" -mutators=cxx_add_to_sub %s.exe 2>&1 | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NO-COVERAGE
// CHECK-NO-COVERAGE:[info] Survived mutants (1/1):
// CHECK-NO-COVERAGE:{{^.*}}main.c:2:5: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:[info] Mutation score: 0%

// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -coverage-info=%s.profdata -linker-flags="-fprofile-instr-generate" -mutators=cxx_add_to_sub %s.exe 2>&1 | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// CHECK-COVERAGE:[info] No mutants found. Mutation score: infinitely high
