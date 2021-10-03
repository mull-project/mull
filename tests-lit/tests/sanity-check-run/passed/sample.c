int main() {
  return 0;
}

// clang-format off

// RUN: cd / && %clang_cc %sysroot -fembed-bitcode %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: (unset TERM; %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" -workers=1 %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] No mutants found. Mutation score: infinitely high

