
int call_me(int a, int b, int c){
  if(b > c){
    if (a == 0){
      return b + c;
    }else{
      return c-b;
    }
  }
  return a+b;
}

int call_me_too(int j, int k, int l){
  return (j < k) ? k+j:k+l;
}

int main(){

  int r = call_me(1,2,3);
  int m = call_me_too(1,2,3);

  return 0;
}
// clang-format off

// RUN: cd / && %clang_cc %sysroot %s -fembed-bitcode -g -fprofile-instr-generate -fcoverage-mapping -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: env LLVM_PROFILE_FILE=%s.profraw %s.exe
// RUN: %llvm_profdata merge %s.profraw -o %s.profdata

// RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %s-ir-no-cov.exe
// RUN: unset TERM; %mull_cxx -keep-executable -output=%s.mutated.exe -linker=%clang_cc -linker-flags="%sysroot -fprofile-instr-generate" -mutators=cxx_add_to_sub %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NO-COVERAGE
// RUN: unset TERM; %mull_runner %s.mutated.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NO-COVERAGE
// RUN: unset TERM; %mull_runner %s-ir-no-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-NO-COVERAGE
// CHECK-NO-COVERAGE:[info] Survived mutants (4/4):
// CHECK-NO-COVERAGE:{{^.*}}main.c:5:16: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:{{^.*}}main.c:10:11: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:{{^.*}}main.c:14:21: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:{{^.*}}main.c:14:25: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-NO-COVERAGE:[info] Mutation score: 0%

// RUN: cd %S && env MULL_CONFIG=%S/mull.cov.yml %clang_cc %sysroot %s %pass_mull_ir_frontend -g -fprofile-instr-generate -fcoverage-mapping -o %s-ir-cov.exe
// RUN: unset TERM; %mull_cxx -keep-executable -output=%s.mutated.cov.exe -linker=%clang_cc -coverage-info=%s.profdata -linker-flags="%sysroot -fprofile-instr-generate" -mutators=cxx_add_to_sub %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// LLVM 9 and below seems to lose the executable bit when copying the file, adding it here explicitly
// RUN: chmod +x %s.mutated.cov.exe
// RUN: unset TERM; %mull_runner %s.mutated.cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// RUN: unset TERM; %mull_runner %s-ir-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-COVERAGE
// CHECK-COVERAGE:[info] Survived mutants (2/4):
// CHECK-COVERAGE:{{^.*}}main.c:10:11: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:{{^.*}}main.c:14:21: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:[info] Not Covered mutants (2/4):
// CHECK-COVERAGE:{{^.*}}main.c:5:16: warning: Not Covered: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:{{^.*}}main.c:14:25: warning: Not Covered: Replaced + with - [cxx_add_to_sub]
// CHECK-COVERAGE:[info] Mutation score: 0%
