#ifndef FLAG
#error "FLAG is not defined"
#endif

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
RUN: cd %CURRENT_DIR
RUN: cd / && env MULL_CONFIG=%S/mull.no_junk.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g -DFLAG=1 %s -o %s-ir-no-junk.exe
RUN: %mull_runner -reporters=IDE %s-ir-no-junk.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-JUNK-DETECTION

WITHOUT-JUNK-DETECTION:{{^.*}}sample.cpp:5:13: warning: Survived: Removed the call to the function [cxx_remove_void_call]{{$}}

RUN: cd %S && env MULL_CONFIG=%S/mull.no_flag.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g -DFLAG=1 %s -o %s-ir-no-flag.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-NO-FLAG-MUTATE
RUN: %mull_runner -ide-reporter-show-killed %s-ir-no-flag.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-NO-FLAG

WITH-JUNK-DETECTION-NO-FLAG-MUTATE:{{^.*}}sample.cpp:2:2: error: "FLAG is not defined"
WITH-JUNK-DETECTION-NO-FLAG-MUTATE:#error "FLAG is not defined"
WITH-JUNK-DETECTION-NO-FLAG-MUTATE: ^
WITH-JUNK-DETECTION-NO-FLAG-MUTATE:[warning] Cannot parse file: '{{.*}}sample.cpp':
WITH-JUNK-DETECTION-NO-FLAG-MUTATE:mull {{.*}}sample.cpp{{$}}
WITH-JUNK-DETECTION-NO-FLAG-MUTATE:Make sure that the flags provided to Mull are the same flags that are used for normal compilation.
TODO: It is interesting why there is no junk even if we have the error above.
WITH-JUNK-DETECTION-NO-FLAG:[info] Killed mutants (1/1):

RUN: cd %S && env MULL_CONFIG=%S/mull.with_flag.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g -DFLAG=1 %s -o %s-ir-with-flag.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-WITH-FLAG-MUTATE
RUN: %mull_runner -reporters=IDE -ide-reporter-show-killed %s-ir-with-flag.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-WITH-FLAG

WITH-JUNK-DETECTION-WITH-FLAG-MUTATE-NOT:#error "FLAG is not defined"
WITH-JUNK-DETECTION-WITH-FLAG-MUTATE-NOT:{{^.*[Ee]rror.*$}}
WITH-JUNK-DETECTION-WITH-FLAG:[info] Killed mutants (1/1):
**/
