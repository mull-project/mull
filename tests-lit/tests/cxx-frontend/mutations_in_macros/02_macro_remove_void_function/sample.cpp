extern "C" {
extern int printf(const char *, ...);
}

void voidSum(int a, int b, int *result) {
  *result = a + b;
}

#define VOID_SUM(a, b, result) voidSum(a, b, result)

int sum(int a, int b) {
  int result = 0;
  VOID_SUM(a, b, &result);
  return result;
}

int main() {
  if (sum(2, 3) == 5) {
    printf("NORMAL\n");
    return 0;
  } else {
    printf("MUTATED\n");
    return 1;
  }
}

// clang-format off

/**
RUN: %clang_cxx %sysroot -fplugin=%mull_frontend_cxx %s -o %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=FRONTEND
FRONTEND:Recording mutation point: cxx_remove_void_call:{{.*}}/sample.cpp:9:32 (end: 9:53)

RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITHOUT_MUTATION
RUN: (env "cxx_remove_void_call:%s:9:32"=1 %s.exe || true) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITH_MUTATION

STANDALONE_WITHOUT_MUTATION:NORMAL
STANDALONE_WITH_MUTATION:MUTATED

RUN: %mull_runner %s.exe -ide-reporter-show-killed | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=MULL_RUNNER

MULL_RUNNER:[info] Killed mutants (1/1):
MULL_RUNNER:{{.*}}sample.cpp:9:32: warning: Killed: Removed the call to the function [cxx_remove_void_call]
*/
