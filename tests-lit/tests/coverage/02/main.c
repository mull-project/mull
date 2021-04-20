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

// RUN: unset TERM; %MULL_EXEC -keep-executable -output=%s.mutated.exe -linker=%clang_cc -coverage-info=%s.profdata -ide-reporter-show-killed -include-not-covered -linker-flags="-fprofile-instr-generate" -mutators=cxx_add_to_sub %s.exe 2>&1 | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -include-not-covered %s.mutated.exe 2>&1 | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Not Covered mutants (1/1):
// CHECK:{{^.*}}main.c:2:5: warning: Not Covered: Replaced + with - [cxx_add_to_sub]
// CHECK:[info] Mutation score: 0%
