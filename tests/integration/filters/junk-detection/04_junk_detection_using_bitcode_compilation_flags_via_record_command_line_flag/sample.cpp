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
RUN: cd / && env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot %pass_mull_ir_frontend -g -DFLAG=1 -grecord-command-line %s -o %s-ir-record-cli.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-RECORD-COMMAND-LINE-MUTATE
RUN: (unset TERM; %mull_runner -reporters=IDE -ide-reporter-show-killed %s-ir-record-cli.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-RECORD-COMMAND-LINE
WITH-RECORD-COMMAND-LINE-MUTATE:[info] Found compilation flags in the input bitcode
WITH-RECORD-COMMAND-LINE-MUTATE-NOT:{{^.*[Ee]rror.*$}}
WITH-RECORD-COMMAND-LINE:[info] Killed mutants (1/1):
**/
