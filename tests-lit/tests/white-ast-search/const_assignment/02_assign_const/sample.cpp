/// TODO: IDE reporter reports location of mutation instruction which is '+'
/// but we would rather want to see the location of '5'.
int foo() {
  int var;
  var = 0;
  return var;
}

int main() {
  return foo() != 0;
}

/**
; RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
; RUN: cd %CURRENT_DIR
; RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
; RUN: (unset TERM; %MULL_EXEC -debug -enable-ast -test-framework CustomTest -mutators=cxx_assign_const -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines
; CHECK-NOT:{{^.*[Ee]rror.*$}}
; CHECK-NOT:{{^.*[Ww]arning.*$}}

; CHECK:[info] AST Search: looking for mutations in the source files (threads: 1)
; CHECK:[debug] AST Search: recording mutation "Assign Const": {{.*}}sample.cpp:5:7

; CHECK:[info] Applying filter: AST mutation filter (threads: 1)
; CHECK:[debug] ASTMutationFilter: whitelisting mutation "Assign Const": {{.*}}sample.cpp:5:7

; CHECK:[info] Applying filter: junk (threads: 1)
; CHECK:[debug] ASTMutationStorage: recording mutation "Assign Const": {{.*}}sample.cpp:5:7

; CHECK:[info] Killed mutants (1/1):
; CHECK:{{^.*}}sample.cpp:5:7: warning: Killed: Replaced 'a = b' with 'a = 42' [cxx_assign_const]{{$}}
; CHECK:  var = 0;
; CHECK:      ^
; CHECK:[info] Mutation score: 100%
; CHECK-EMPTY:
**/
