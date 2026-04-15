int sub(int a, int b) {
  return a + b;
}

int main() {
  return sub(-2, 2);
}

// clang-format off

/**
RUN: %clang_cxx %sysroot -O0 -c -emit-llvm -g -grecord-command-line %s -o %s.bc
RUN: cp %s.bc %s-copy.bc

RUN: %mull_instrument %s.bc -o %s-mutated.bc
RUN: diff %s.bc %s-copy.bc
RUN: %clang_cxx %sysroot %s-mutated.bc -o %s.exe
RUN: unset TERM; %mull_runner -reporters=IDE %s.exe | %filecheck %s --dump-input=fail --check-prefix=MULL_RUNNER

RUN: %clang_cxx %sysroot -O0 -c -emit-llvm -g -grecord-command-line %s -o - | %mull_instrument | %clang_cxx %sysroot -x ir - -o %s-pipe.exe
RUN: unset TERM; %mull_runner -reporters=IDE %s-pipe.exe | %filecheck %s --dump-input=fail --check-prefix=MULL_RUNNER

MULL_RUNNER:[info] Mutation score: 100%
MULL_RUNNER:[info] All mutations have been killed
MULL_RUNNER:[info] Total execution time: {{.*}}
MULL_RUNNER-EMPTY:
*/
