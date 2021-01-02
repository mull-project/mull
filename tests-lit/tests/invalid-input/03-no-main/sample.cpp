// clang-format off

void shared_lib() {}

/**
RUN: cd / && %CLANG_EXEC %s -fembed-bitcode -shared -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %MULL_EXEC -linker=%clang_cxx %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[info] No mutants found. Mutation score: infinitely high
**/


