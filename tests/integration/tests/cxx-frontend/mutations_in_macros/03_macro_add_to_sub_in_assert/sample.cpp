extern "C" {
extern int printf(const char *, ...);
extern void exit(int);
}

#define ASSERT(expr) \
    (!(expr)) ? ((void)printf("MUTATED\n", #expr), exit(1)) : (void)0

int test() {
  ASSERT((2 + 3) == 5);
  return 0;
}

int main() {
  if (test() == 0) {
    printf("NORMAL\n");
    return 0;
  }
  printf("Should not reach here\n");
  return 1;
}

// clang-format off

/**
RUN: %clang_cxx %sysroot -fplugin=%mull_frontend_cxx %s -o %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=FRONTEND
FRONTEND:Recording mutation point: cxx_add_to_sub:{{.*}}/sample.cpp:10:13 (end: 10:14)

RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITHOUT_MUTATION
RUN: (env "cxx_add_to_sub:%s:10:13"=1 %s.exe || true) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITH_MUTATION

STANDALONE_WITHOUT_MUTATION:NORMAL
STANDALONE_WITH_MUTATION:MUTATED

RUN: %mull_runner %s.exe -ide-reporter-show-killed | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=MULL_RUNNER

MULL_RUNNER:[info] Killed mutants (1/1):
MULL_RUNNER:{{.*}}sample.cpp:10:13: warning: Killed: Replaced + with - [cxx_add_to_sub]
*/
