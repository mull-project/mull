/**
; RUN: cd / && %CLANG_EXEC %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines
; CHECK:{{.*No bitcode: x86_64.*}}
; CHECK:No mutants found. Mutation score: infinitely high
**/

int main() {
  return 0;
}
