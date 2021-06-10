extern "C" {
extern int printf(const char *, ...);
}

double plus10(double arg) {
  double base = 10.0;
  return base + arg;
}

int main() {
  if (plus10(5.0) == 15.0) {
    printf("NORMAL\n");
    return 0;
  } else {
    printf("MUTATED\n");
    return 1;
  }
}

// clang-format off

/**
RUN: %CLANG_EXEC -fplugin=%mull_frontend_cxx -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_init_const %s -o %s.exe

RUN: %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITHOUT_MUTATION
RUN: (env "cxx_init_const:%s:6:17"=1 %s.exe || true) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=STANDALONE_WITH_MUTATION

STANDALONE_WITHOUT_MUTATION:NORMAL
STANDALONE_WITH_MUTATION:MUTATED

RUN: %mull_runner %s.exe -ide-reporter-show-killed | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=MULL_RUNNER

MULL_RUNNER:[info] Killed mutants (1/1):
MULL_RUNNER:{{.*}}sample.cpp:6:17: warning: Killed: Replaced 'T a = b' with 'T a = 42' [cxx_init_const]
*/
