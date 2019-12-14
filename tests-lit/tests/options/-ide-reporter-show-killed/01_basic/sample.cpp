/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
; RUN: cd %CURRENT_DIR

; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION)
; WITHOUT-OPTION:{{^Running mutants \(threads: 2\): 2\/2.*$}}
; WITHOUT-OPTION:Survived mutants (1/2):
; WITHOUT-OPTION-EMPTY:
; WITHOUT-OPTION-NEXT:{{^.*}}sample.cpp:35:19: warning: Survived: Replaced + with -{{$}}
; WITHOUT-OPTION-NEXT:  result = result + 0;
; WITHOUT-OPTION-NEXT:                  ^
; WITHOUT-OPTION-EMPTY:
; WITHOUT-OPTION-NEXT:Mutation score: 50%
; WITHOUT-OPTION-EMPTY:

; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -reporters=IDE -ide-reporter-show-killed %s.exe | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION)
; WITH-OPTION:{{^Running mutants \(threads: 2\): 2\/2.*$}}
; WITH-OPTION:Killed mutants (1/2):
; WITH-OPTION-EMPTY:
; WITH-OPTION-NEXT:{{^.*}}sample.cpp:34:18: warning: Killed: Replaced + with -
; WITH-OPTION-NEXT:  int result = a + b;
; WITH-OPTION-NEXT:                 ^
; WITH-OPTION-EMPTY:
; WITH-OPTION-NEXT:Survived mutants (1/2):
; WITH-OPTION-EMPTY:
; WITH-OPTION-NEXT:{{^.*}}sample.cpp:35:19: warning: Survived: Replaced + with -
; WITH-OPTION-NEXT:  result = result + 0;
; WITH-OPTION-NEXT:                  ^
; WITH-OPTION-EMPTY:
; WITH-OPTION-NEXT:Mutation score: 50%
*/

int sum(int a, int b) {
  int result = a + b;
  result = result + 0;
  return result;
}

int main() {
  return sum(-2, 2);
}
