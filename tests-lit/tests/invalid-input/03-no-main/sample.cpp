// clang-format off

void shared_lib() {}

/**
RUN: cd / && %CLANG_EXEC %s -fembed-bitcode -shared -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %MULL_EXEC -test-framework CustomTest %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[warning] No tests found. Either switch to CustomTest, or ensure that the executable contains bitcode for all source files.
CHECK:[info] No mutants found. Mutation score: infinitely high
**/


