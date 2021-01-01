int callee() {
  return 0;
}

int caller() {
  return callee();
}

int main() {
  return caller() != 0;
}

// clang-format off

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cxx -workers=1 -debug -enable-ast -test-framework CustomTest -mutators=replace_call_mutator -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] AST Search: looking for mutations in the source files (threads: 1)
CHECK:[debug] AST Search: recording mutation "Replace Call": {{.*}}sample.cpp:6:10

CHECK:[info] Applying filter: AST mutation filter (threads: 1)
CHECK:[debug] ASTMutationFilter: whitelisting mutation "Replace Call": {{.*}}sample.cpp:6:10

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] ASTMutationStorage: recording mutation "Replace Call": {{.*}}sample.cpp:6:10

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:6:10: warning: Killed: Replace Call: replaced a call to function _Z6calleev with 42 [replace_call_mutator]{{$}}
CHECK:  return callee();
CHECK:         ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
