// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot %pass_mull_ir_frontend -g -O0 %s -o %s-ir.exe
RUN: (unset TERM; %mull_runner %s-ir.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[info] No mutants found. Mutation score: infinitely high
**/

int main() {
  return 0;
}
