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

RUN: cd / && env MULL_CONFIG=%S/mull.no_junk.yml %clang_cxx %sysroot %pass_mull_ir_frontend -g -DFLAG=1 %s -o %s-ir-no-junk.exe
RUN: (unset TERM; %mull_runner -reporters=IDE -ide-reporter-show-killed %s-ir-no-junk.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-RECORD-COMMAND-LINE
WITHOUT-RECORD-COMMAND-LINE-MUTATE-NOT:Found compilation flags in the input bitcode
WITHOUT-RECORD-COMMAND-LINE:{{^.*}}sample.cpp:5:13: warning: Survived: Removed the call to the function [cxx_remove_void_call]{{$}}


RUN: cd / && env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot %pass_mull_ir_frontend -g -DFLAG=1 -grecord-command-line %s -o %s-ir-record-cli.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-RECORD-COMMAND-LINE-MUTATE
RUN: (unset TERM; %mull_runner -reporters=IDE -ide-reporter-show-killed %s-ir-record-cli.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-RECORD-COMMAND-LINE
WITH-RECORD-COMMAND-LINE-MUTATE:[info] Found compilation flags in the input bitcode
WITH-RECORD-COMMAND-LINE-MUTATE-NOT:{{^.*[Ee]rror.*$}}
WITH-RECORD-COMMAND-LINE:[info] Killed mutants (1/1):
**/
