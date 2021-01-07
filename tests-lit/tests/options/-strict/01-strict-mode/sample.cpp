int main() {
  return 0;
}

// clang-format off

/**
RUN: cd / && %CLANG_EXEC %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cc %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT-OPTION
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cc -strict %s.exe 2>&1; test $? = 1) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH-OPTION

WITHOUT-OPTION:[warning] No bitcode: x86_64
WITHOUT-OPTION:[info] No mutants found. Mutation score: infinitely high

WITH-OPTION:[info] Diagnostics: Strict Mode enabled. Warning messages will be treated as fatal errors.
WITH-OPTION:[warning] No bitcode: x86_64
WITH-OPTION:[warning] Strict Mode enabled: warning messages are treated as fatal errors. Exiting now.
WITH-OPTION-EMPTY:
**/
