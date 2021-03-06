// clang-format off

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cc %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{.*No bitcode: x86_64.*}}
CHECK:[info] No mutants found. Mutation score: infinitely high
**/

int main() {
  return 0;
}
