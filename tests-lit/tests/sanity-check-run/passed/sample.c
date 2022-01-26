int main() {
  return 0;
}

// clang-format off

// RUN: cd / && %clang_cc %sysroot %pass_mull_ir_frontend %s -o %s.exe
// RUN: (unset TERM; %mull_runner -workers=1 %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] No mutants found. Mutation score: infinitely high

