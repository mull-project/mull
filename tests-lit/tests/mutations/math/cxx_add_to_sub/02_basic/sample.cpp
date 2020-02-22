/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %FILECHECK_EXEC %s
; CHECK:[info] Running mutants (threads: 1)
; CHECK:{{^       \[################################\] 1/1\. Finished .*}}
; CHECK:[info] All mutations have been killed
; CHECK:[info] Mutation score: 100%
; CHECK-EMPTY:
**/

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}
