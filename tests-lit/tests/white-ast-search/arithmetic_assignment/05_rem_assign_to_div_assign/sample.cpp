int div_assign(int a, int b) {
  a %= b;
  return a;
}

int main() {
  return div_assign(5, 2) != 1;
}

// clang-format off

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cxx -debug -enable-ast -mutators=cxx_rem_assign_to_div_assign -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] AST Search: looking for mutations in the source files (threads: 1)
CHECK:[debug] AST Search: recording mutation "Rem-Assign to Div-Assign": {{.*}}sample.cpp:2:5

CHECK:[info] Applying filter: AST mutation filter (threads: 1)
CHECK:[debug] ASTMutationFilter: whitelisting mutation "Rem-Assign to Div-Assign": {{.*}}sample.cpp:2:5

CHECK:[info] Applying filter: junk (threads: 1)
CHECK:[debug] CXXJunkDetector: mutation "Rem-Assign to Div-Assign": {{.*}}sample.cpp:2:5 (end: 2:7)

CHECK:[info] Killed mutants (1/1):
CHECK:{{^.*}}sample.cpp:2:5: warning: Killed: Replaced %= with /= [cxx_rem_assign_to_div_assign]{{$}}
CHECK:  a %= b;
CHECK:    ^
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
