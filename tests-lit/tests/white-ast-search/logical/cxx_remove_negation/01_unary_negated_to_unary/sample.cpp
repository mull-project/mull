bool isFalse(bool a) {
  return !a;
}

int main() {
  return ! (isFalse(true) == false);
}

// clang-format off

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cxx -workers=1 -debug -enable-ast -test-framework CustomTest -mutators=cxx_remove_negation -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] AST Search: looking for mutations in the source files (threads: 1)
CHECK:[debug] AST Search: recording mutation "Remove Unary Negation": {{.*}}sample.cpp:2:10

CHECK:[info] Applying filter: AST mutation filter (threads: 1)
CHECK:[debug] ASTMutationFilter: whitelisting mutation "Remove Unary Negation": {{.*}}sample.cpp:2:10

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] ASTMutationStorage: recording mutation "Remove Unary Negation": {{.*}}sample.cpp:2:10

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:10: warning: Killed: Replaced !a with a [cxx_remove_negation]{{$}}
CHECK:  return !a;
CHECK:         ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
