// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: (unset TERM; %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[warning] No bitcode: x86_64
CHECK:[info] No mutants found. Mutation score: infinitely high
**/

int main() {
  return 0;
}
