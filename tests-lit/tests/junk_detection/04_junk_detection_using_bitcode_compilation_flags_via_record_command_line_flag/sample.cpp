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
REQUIRES: LLVM_8_OR_HIGHER
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -DFLAG=1 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -disable-junk-detection -mutators=cxx_add_to_sub -mutators=cxx_remove_void_call -reporters=IDE -ide-reporter-show-killed %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-RECORD-COMMAND-LINE
WITHOUT-RECORD-COMMAND-LINE-NOT:Found compilation flags in the input bitcode
WITHOUT-RECORD-COMMAND-LINE:{{^.*}}sample.cpp:5:13: warning: Survived: Removed the call to the function [cxx_remove_void_call]{{$}}

RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -DFLAG=1 -grecord-command-line %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -mutators=cxx_remove_void_call -reporters=IDE -ide-reporter-show-killed %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-RECORD-COMMAND-LINE
WITH-RECORD-COMMAND-LINE:[info] Found compilation flags in the input bitcode
WITH-RECORD-COMMAND-LINE-NOT:{{^.*[Ee]rror.*$}}
WITH-RECORD-COMMAND-LINE:[info] Killed mutants (1/1):
**/
