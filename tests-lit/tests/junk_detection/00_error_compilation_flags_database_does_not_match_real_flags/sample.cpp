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
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 -DFLAG=1 %s -o %s.exe
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.no_flag.json.template > %S/compile_commands.no_flag.json
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.with_flag.json.template > %S/compile_commands.with_flag.json
RUN: cd %CURRENT_DIR
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -disable-junk-detection -mutators=cxx_remove_void_call -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-JUNK-DETECTION
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_remove_void_call -mutators=cxx_add_to_sub -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.no_flag.json %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-NO-FLAG
RUN: %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_remove_void_call -mutators=cxx_add_to_sub -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.with_flag.json %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-JUNK-DETECTION-WITH-FLAG

WITHOUT-JUNK-DETECTION:{{^.*}}sample.cpp:5:13: warning: Survived: Removed the call to the function [cxx_remove_void_call]{{$}}

WITH-JUNK-DETECTION-NO-FLAG:{{^.*}}sample.cpp:2:2: error: "FLAG is not defined"
WITH-JUNK-DETECTION-NO-FLAG:#error "FLAG is not defined"
WITH-JUNK-DETECTION-NO-FLAG: ^
WITH-JUNK-DETECTION-NO-FLAG:[warning] Cannot parse file: '{{.*}}sample.cpp':
WITH-JUNK-DETECTION-NO-FLAG:mull-cxx {{.*}}sample.cpp{{$}}
WITH-JUNK-DETECTION-NO-FLAG:Make sure that the flags provided to Mull are the same flags that are used for normal compilation.
TODO: It is interesting why there is no junk even if we have the error above.
WITH-JUNK-DETECTION-NO-FLAG:[info] Killed mutants (1/1):

WITH-JUNK-DETECTION-WITH-FLAG-NOT:#error "FLAG is not defined"
WITH-JUNK-DETECTION-WITH-FLAG-NOT:{{^.*[Ee]rror.*$}}
WITH-JUNK-DETECTION-WITH-FLAG:[info] Killed mutants (1/1):
**/
