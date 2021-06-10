extern "C" {
extern int printf(const char *, ...);
}

void voidSum(int a, int b, int *result) {
  *result = a + b;
}

int sum(int a, int b) {
  int result = 0;
  voidSum(a, b, &result);
  return result;
}

int main() {
  if (sum(2, 2) == 4) {
    printf("NORMAL\n");
    return 0;
  } else {
    printf("MUTATED\n");
    return 1;
  }
}

// clang-format off

/**
RUN: %CLANG_EXEC -fplugin=%mull_frontend_cxx -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_remove_void_call %s -o %s.exe

RUN: %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITHOUT_MUTATION
RUN: (env "cxx_remove_void_call:%s:11:3"=1 %s.exe || true) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITH_MUTATION

STANDALONE_WITHOUT_MUTATION:NORMAL
STANDALONE_WITH_MUTATION:MUTATED

RUN: %mull_runner %s.exe -ide-reporter-show-killed | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=MULL_RUNNER

MULL_RUNNER:[info] Killed mutants (1/1):
MULL_RUNNER:{{.*}}sample.cpp:11:3: warning: Killed: Removed the call to the function [cxx_remove_void_call]
*/
