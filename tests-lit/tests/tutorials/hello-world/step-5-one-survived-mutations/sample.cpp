/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -ide-reporter-show-killed %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines
; CHECK:Killed mutants (1/2):
; CHECK:{{^.*}}sample.cpp:13:11: warning: Killed: Replaced >= with <{{$}}
; CHECK:Survived mutants (1/2):
; CHECK:{{^.*}}sample.cpp:13:11: warning: Survived: Replaced >= with >{{$}}
; CHECK:Mutation score: 50%
**/

bool valid_age(int age) {
  if (age >= 21) {
    return true;
  }
  return false;
}

int main() {
  bool test1 = valid_age(25) == true;
  if (!test1) {
    /// test failed
    return 1;
  }

  bool test2 = valid_age(20) == false;
  if (!test2) {
    /// test failed
    return 1;
  }

  /// success
  return 0;
}
