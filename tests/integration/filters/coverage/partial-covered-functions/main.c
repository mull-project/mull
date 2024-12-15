
int call_me(int a, int b, int c) {
  if (b > c) {
    if (a == 0) {
      return b + c;
    } else {
      return c - b;
    }
  }
  return a + b;
}

int call_me_too(int j, int k, int l) {
  return (j < k) ? k + j : k + l;
}

int main() {

  int r = call_me(1, 2, 3);
  int m = call_me_too(1, 2, 3);

  return 0;
}
// clang-format off
// RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -o %s.exe
// RUN: unset TERM; %mull_runner --allow-surviving %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NO-COVERAGE
// CHECK-NO-COVERAGE:[info] Survived mutants (4/4):
// CHECK-NO-COVERAGE:{{^.*}}main.c:5:16: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:{{^.*}}main.c:10:12: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:{{^.*}}main.c:14:22: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:{{^.*}}main.c:14:30: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:[info] Mutation score: 0%

// RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %s-cov.exe
// RUN: unset TERM; %mull_runner --allow-surviving -include-not-covered %s-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// RUN: env LLVM_PROFILE_FILE=%s.profraw %s-cov.exe
// RUN: %llvm_profdata merge %s.profraw -o %s.profdata
// RUN: unset TERM; %mull_runner --allow-surviving -coverage-info %s.profdata -include-not-covered %s-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// CHECK-COVERAGE:[info] Survived mutants (2/4):
// CHECK-COVERAGE:{{^.*}}main.c:10:12: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:{{^.*}}main.c:14:22: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:[info] Not Covered mutants (2/4):
// CHECK-COVERAGE:{{^.*}}main.c:5:16: warning: Not Covered: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:{{^.*}}main.c:14:30: warning: Not Covered: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:[info] Mutation score: 0%
