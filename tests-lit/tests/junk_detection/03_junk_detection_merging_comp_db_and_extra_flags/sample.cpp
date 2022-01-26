#ifndef FLAG_VIA_COMP_DB
#error "FLAG_VIA_COMP_DB is not defined"
#endif

#ifndef FLAG_VIA_EXTRA_FLAGS
#error "FLAG_VIA_EXTRA_FLAGS is not defined"
#endif

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

// clang-format off

/**
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %CURRENT_DIR

RUN: cd / && env MULL_CONFIG=%S/mull.no_junk.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g -DFLAG_VIA_COMP_DB=1 -DFLAG_VIA_EXTRA_FLAGS=1 %s -o %s-ir-no-junk.exe
RUN: %mull_runner %s-ir-no-junk.exe -reporters=IDE | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-JUNK-DETECTION

WITHOUT-JUNK-DETECTION:{{^.*}}sample.cpp:9:13: warning: Survived: Removed the call to the function [cxx_remove_void_call]{{$}}

RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g -DFLAG_VIA_COMP_DB=1 -DFLAG_VIA_EXTRA_FLAGS=1 %s -o %s-ir-with-flags.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-WITH-FLAGS-MUTATE
RUN: %mull_runner %s-ir-with-flags.exe -reporters=IDE -ide-reporter-show-killed 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-WITH-FLAGS

WITH-JUNK-DETECTION-WITH-FLAGS-MUTATE-NOT:{{^.*[Ee]rror.*$}}
WITH-JUNK-DETECTION-WITH-FLAGS:[info] Killed mutants (1/1):
**/
