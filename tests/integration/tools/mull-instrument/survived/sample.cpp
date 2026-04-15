extern "C" {
extern int printf(const char *, ...);
}

int sum(int a, int b) {
  return a + b;
}

int main() {
  if (sum(2, 0) == 2) {
    printf("NORMAL\n");
    return 0;
  } else {
    printf("MUTATED\n");
    return 1;
  }
}

// clang-format off

/**
RUN: %clang_cxx %sysroot -O0 -c -emit-llvm -g -grecord-command-line %s -o %s.bc
RUN: %mull_instrument %s.bc -o %s-mutated.bc
RUN: %clang_cxx %sysroot %s-mutated.bc -o %s.exe

RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITHOUT_MUTATION
RUN: (env "cxx_add_to_sub:$(realpath %s):6:12:6:13"=1 %s.exe || true) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITHOUT_MUTATION

STANDALONE_WITHOUT_MUTATION:NORMAL

RUN: unset TERM; %mull_runner --allow-surviving %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=MULL_RUNNER

MULL_RUNNER:[info] Survived mutants (1/1):
MULL_RUNNER-NEXT:{{^.*}}sample.cpp:6:12: warning: Survived: Replaced + with - [cxx_add_to_sub]
MULL_RUNNER-NEXT:  return a + b;
MULL_RUNNER-NEXT:           ^
MULL_RUNNER-NEXT:[info] Mutation score: 0%
*/
