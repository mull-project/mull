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
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 -DFLAG_VIA_COMP_DB=1 -DFLAG_VIA_EXTRA_FLAGS=1 %s -o %s.exe
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: cd %CURRENT_DIR
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -disable-junk-detection -mutators=cxx_add_to_sub -mutators=cxx_remove_void_call -reporters=IDE %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-JUNK-DETECTION
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -mutators=cxx_remove_void_call -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json -compilation-flags '-DFLAG_VIA_EXTRA_FLAGS=1' %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-WITH-FLAGS

WITHOUT-JUNK-DETECTION:{{^.*}}sample.cpp:9:13: warning: Survived: Removed the call to the function [cxx_remove_void_call]{{$}}

WITH-JUNK-DETECTION-WITH-FLAGS-NOT:{{^.*[Ee]rror.*$}}
WITH-JUNK-DETECTION-WITH-FLAGS:[info] Killed mutants (1/1):
**/
