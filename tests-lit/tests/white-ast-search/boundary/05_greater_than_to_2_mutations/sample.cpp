int greater_than(int a, int b) {
  return a > b;
}

int main() {
  return ! (greater_than(3, 3) == 0);
}

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: sed -e "s:%PWD:%S:g" %S/compile_commands.json.template > %S/compile_commands.json
RUN: (unset TERM; %MULL_EXEC -debug -enable-ast -test-framework CustomTest -mutators=cxx_gt_to_ge -mutators=cxx_gt_to_le -reporters=IDE -ide-reporter-show-killed -compdb-path %S/compile_commands.json %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --strict-whitespace --match-full-lines
CHECK-NOT:{{^.*[Ee]rror.*$}}
CHECK-NOT:{{^.*[Ww]arning.*$}}

CHECK:[info] AST Search: looking for mutations in the source files (threads: 1)
CHECK:[debug] AST Search: recording mutation "Greater Than to Less Or Equal": {{.*}}sample.cpp:2:12
CHECK:[debug] AST Search: recording mutation "Greater Than to Greater or Equal": {{.*}}sample.cpp:2:12

CHECK:[info] Applying filter: AST mutation filter (threads: 2)

NOTE: These lines we cannot test because they happen in a random order
[debug] ASTMutationFilter: whitelisting mutation "Greater Than to Greater or Equal": {{.*}}sample.cpp:2:12
[debug] ASTMutationFilter: whitelisting mutation "Greater Than to Less Or Equal": {{.*}}sample.cpp:2:12

CHECK:[info] Applying filter: junk (threads: 2)
CHECK:[debug] ASTMutationStorage: recording mutation "Greater Than to Greater or Equal": {{.*}}sample.cpp:2:12

CHECK:[info] Killed mutants (2/2):
CHECK:{{^.*}}sample.cpp:2:12: warning: Killed: Replaced > with >= [cxx_gt_to_ge]{{$}}
CHECK:  return a > b;
CHECK:           ^
CHECK:{{^.*}}sample.cpp:2:12: warning: Killed: Replaced > with <= [cxx_gt_to_le]{{$}}
CHECK:  return a > b;
CHECK:           ^
CHECK:[info] Mutation score: 100%
CHECK-EMPTY:
**/
