/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: %MULL_EXEC -test-framework CustomTest -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %FILECHECK_EXEC %s
; CHECK: Running mutants (threads: 1): 1/1
; CHECK: Survived mutants (0/1):
; CHECK: Mutation score: 100%
**/

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}
