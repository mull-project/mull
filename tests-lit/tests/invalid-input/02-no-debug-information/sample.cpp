/**
; /// Without debug information we see the error message.
; RUN: cd / && %CLANG_EXEC -fembed-bitcode %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITHOUT-DEBUG
; WITHOUT-DEBUG:[warning] Bitcode module does not have debug information.
; WITHOUT-DEBUG:[info] No mutants found. Mutation score: infinitely high

; /// With debug information we do not see the error message.
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines --check-prefix=WITH-DEBUG
; WITH-DEBUG-NOT:[warning] Bitcode module does not have debug information.
; WITH-DEBUG:[info] Running mutants (threads: 1)
; WITHOUT-OPTION:{{^       \[################################\] 1/1\. Finished .*}}
; WITH-DEBUG:[info] Mutation score: 100%
**/

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}
