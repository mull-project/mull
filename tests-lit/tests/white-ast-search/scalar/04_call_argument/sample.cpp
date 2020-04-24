int dummy(int a, int b) {
  return a + b;
}

int caller() {
  return dummy(0, 0);
}

int main() {
  return caller();
}

/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
; RUN: (unset TERM; %MULL_EXEC -debug -enable-ast -test-framework CustomTest -mutators=scalar_value_mutator -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines
; CHECK-NOT:{{^.*[Ee]rror.*$}}
; CHECK-NOT:{{^.*[Ww]arning.*$}}

; CHECK:[info] AST Search: looking for mutations in the source files (threads: 1)
; CHECK:[debug] AST Search: recording mutation "Scalar Value": {{.*}}sample.cpp:6:10
; CHECK:[debug] AST Search: recording mutation "Scalar Value": {{.*}}sample.cpp:6:10

; CHECK:[info] Applying filter: AST mutation filter (threads: 2)
; CHECK:[debug] ASTMutationFilter: whitelisting mutation "Scalar Value": {{.*}}sample.cpp:6:10
; CHECK:[debug] ASTMutationFilter: whitelisting mutation "Scalar Value": {{.*}}sample.cpp:6:10

; CHECK:[info] Applying filter: junk (threads: 2)
; CHECK:[debug] ASTMutationStorage: recording mutation "Scalar Value": {{.*}}sample.cpp:6:10
; CHECK:[debug] ASTMutationStorage: recording mutation "Scalar Value": {{.*}}sample.cpp:6:10

; CHECK:[info] Killed mutants (2/2):
; CHECK:{{^.*}}sample.cpp:6:10: warning: Killed: Replacing scalar with 0 or 42 [scalar_value_mutator]{{$}}
; CHECK:  return dummy(0, 0);
; CHECK:         ^
; CHECK:{{^.*}}sample.cpp:6:10: warning: Killed: Replacing scalar with 0 or 42 [scalar_value_mutator]{{$}}
; CHECK:  return dummy(0, 0);
; CHECK:         ^
; CHECK:[info] Mutation score: 100%
; CHECK-EMPTY:
**/
