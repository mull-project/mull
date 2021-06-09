extern "C" {
extern int printf(const char *, ...);
}

int minus(int arg) {
  return -arg;
}

int main() {
  if (minus(5) == -5) {
    printf("NORMAL\n");
    return 0;
  } else {
    printf("MUTATED\n");
    return 1;
  }
}

// clang-format off

/**
RUN: %CLANG_EXEC -fplugin=%mull_frontend_cxx -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_minus_to_noop %s -o %s.exe

RUN: %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITHOUT_MUTATION
RUN: (env "cxx_minus_to_noop:%s:6:10"=1 %s.exe || true) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITH_MUTATION

STANDALONE_WITHOUT_MUTATION:NORMAL
STANDALONE_WITH_MUTATION:MUTATED

RUN: %mull_runner %s.exe -ide-reporter-show-killed | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=MULL_RUNNER

MULL_RUNNER:[info] Killed mutants (1/1):
MULL_RUNNER:{{.*}}sample.cpp:6:10: warning: Killed: Replaced -x with x [cxx_minus_to_noop]
*/
