// clang-format off

void shared_lib() {}

/**
RUN: cd / && %clang_cxx %sysroot %s -fembed-bitcode -shared -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[info] No mutants found. Mutation score: infinitely high
**/


