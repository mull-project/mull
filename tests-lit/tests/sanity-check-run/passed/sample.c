int main() {
  return 0;
}

// clang-format off

// RUN: cd / && %clang_cc -fembed-bitcode %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: (unset TERM; %MULL_EXEC -linker=%clang_cc -workers=1 %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] No mutants found. Mutation score: infinitely high

