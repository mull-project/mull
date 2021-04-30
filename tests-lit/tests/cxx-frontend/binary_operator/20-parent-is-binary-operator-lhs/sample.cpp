extern "C" {
extern int printf(const char *, ...);
}

int sum_times() {
  for (int i = 0; i < 10; i = i + 1) {
    if (i < 0) {
      return 0;
    }
  }
  return 1;
}

int main() {
  if (sum_times() == 1) {
    printf("NORMAL\n");
  } else {
    printf("MUTATED\n");
  }
  return 0;
}

// clang-format off

/**
RUN: %CLANG_EXEC -Xclang -load -Xclang %mull_frontend_cxx -Xclang -add-plugin -Xclang mull-cxx-frontend -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_add_to_sub %s -o %s.exe
RUN: %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT_MUTATION
RUN: FOO=1 %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH_MUTATION

WITHOUT_MUTATION:NORMAL
WITH_MUTATION:MUTATED
*/
