int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
RUN: sed -e "s:%PWD:%s:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %CURRENT_DIR
RUN: env MULL_CONFIG=%S/mull.no_junk.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir-no-junk.exe
RUN: %mull_runner -workers=1 -reporters=IDE %s-ir-no-junk.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-JUNK-DETECTION

RUN: env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
RUN: %mull_runner -workers=1 -reporters=IDE -ide-reporter-show-killed %s-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION

WITHOUT-JUNK-DETECTION:{{^.*}}[info] Running mutants (threads: 1){{$}}
WITHOUT-JUNK-DETECTION:{{^.*}}sample.cpp:1:13: warning: Survived: Removed the call to the function [cxx_remove_void_call]{{$}}
WITHOUT-JUNK-DETECTION-NOT:[info] Mutation score: 100%

WITH-JUNK-DETECTION:{{^.*}}[info] Running mutants (threads: 1){{$}}
WITH-JUNK-DETECTION:[info] Killed mutants (1/1):
WITH-JUNK-DETECTION:{{^.*}}/sample.cpp:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]{{$}}
WITH-JUNK-DETECTION:  return a + b;
WITH-JUNK-DETECTION:           ^
WITH-JUNK-DETECTION:[info] All mutations have been killed
WITH-JUNK-DETECTION:[info] Mutation score: 100%
**/
