#include <stdarg.h>
int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/Output/compile_commands.json.temp
RUN: sed -e "s:%CC:%clang_cc:g" %S/Output/compile_commands.json.temp > %S/compile_commands.json

RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK_MUTATE
CHECK_MUTATE-NOT:{{.*}}: fatal error: 'stdarg.h' file not found

RUN: %mull_runner -workers=1 -reporters=IDE -ide-reporter-show-killed %s-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION

WITH-JUNK-DETECTION:{{^.*}}[info] Running mutants (threads: 1){{$}}
WITH-JUNK-DETECTION:[info] Killed mutants (1/1):
WITH-JUNK-DETECTION:{{^.*}}/sample.cpp:3:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
WITH-JUNK-DETECTION:  return a + b;
WITH-JUNK-DETECTION:           ^
WITH-JUNK-DETECTION:[info] All mutations have been killed
WITH-JUNK-DETECTION:[info] Mutation score: 100%
**/
