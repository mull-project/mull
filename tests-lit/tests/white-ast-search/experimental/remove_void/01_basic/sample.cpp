static int globalVar = -1;
void voidFunction() {
  globalVar = 0;
}

void foo() {
  voidFunction();
};

int main() {
  foo();
  return globalVar;
}

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %MULL_EXEC -debug -enable-ast -test-framework CustomTest -mutators=remove_void_function_mutator -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] AST Search: looking for mutations in the source files (threads: 1)
CHECK:[debug] AST Search: recording mutation "Remove Void": {{.*}}sample.cpp:7:3

CHECK:[info] Applying filter: AST mutation filter (threads: 1)
CHECK:[debug] ASTMutationFilter: whitelisting mutation "Remove Void": {{.*}}sample.cpp:7:3

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] ASTMutationStorage: recording mutation "Remove Void": {{.*}}sample.cpp:7:3

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:7:3: warning: Killed: Remove Void Call: removed _Z12voidFunctionv [remove_void_function_mutator]{{$}}
CHECK:  voidFunction();
CHECK:  ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
