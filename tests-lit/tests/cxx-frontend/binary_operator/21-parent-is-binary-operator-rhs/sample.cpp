extern "C" {
extern int printf(const char *, ...);
}

int sum_times() {
  return (1 * (1 + 1));
}

int main() {
  if (sum_times() == 2) {
    printf("NORMAL\n");
  } else {
    printf("MUTATED\n");
  }
  return 0;
}

// clang-format off

/**
RUN: %CLANG_EXEC -Xclang -load -Xclang %mull_frontend_cxx -Xclang -add-plugin -Xclang mull-cxx-frontend -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_add_to_sub %s -o %s.exe %mull_rt -Wl,-rpath,%mull_path_to_rt
RUN: %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT_MUTATION
RUN: MULL="cxx_add_to_sub:%s:6:18:1" %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH_MUTATION

WITHOUT_MUTATION:NORMAL
WITH_MUTATION:MUTATED
*/
