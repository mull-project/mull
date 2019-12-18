/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
; RUN: cd %CURRENT_DIR

/// IDEReporter is enabled by default. So we test both invocations of mull-cxx:
/// 1) when -reporters=IDE is not provided
/// 2) when -reporters=IDE is provided

; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub %s.exe | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION)
; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION)
; WITHOUT-OPTION:[info] Running mutants (threads: 1)
; WITHOUT-OPTION:{{^       \[################################\] 1/1\. Finished .*}}
; WITHOUT-OPTION:[info] Survived mutants (1/1):
; WITHOUT-OPTION-NEXT:{{^.*}}sample.cpp:33:18: warning: Survived: Replaced + with -
; WITHOUT-OPTION-NEXT:  int result = a + b;
; WITHOUT-OPTION-NEXT:                 ^
; WITHOUT-OPTION-NEXT:[info] Mutation score: 0%
; WITHOUT-OPTION-EMPTY:

; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -ide-reporter-show-killed %s.exe | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION)
; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -reporters=IDE -ide-reporter-show-killed %s.exe | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION)
; WITH-OPTION:[info] Running mutants (threads: 1)
; WITH-OPTION:{{^       \[################################\] 1/1\. Finished .*}}
; WITH-OPTION:[info] Survived mutants (1/1):
; WITH-OPTION-NEXT:{{^.*}}sample.cpp:33:18: warning: Survived: Replaced + with -
; WITH-OPTION-NEXT:  int result = a + b;
; WITH-OPTION-NEXT:                 ^
; WITH-OPTION-NEXT:[info] Mutation score: 0%
; WITH-OPTION-EMPTY:
*/

int sum(int a, int b) {
  int result = a + b;
  return result;
}

int main() {
  return sum(2, 0) != 2;
}
