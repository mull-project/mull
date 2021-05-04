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
  } else {
    printf("MUTATED\n");
  }
  return 0;
}

// clang-format off

/**
RUN: %CLANG_EXEC -Xclang -load -Xclang %mull_frontend_cxx -Xclang -add-plugin -Xclang mull-cxx-frontend -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_remove_void_call  %s -o %s.exe %mull_rt -Wl,-rpath,%mull_path_to_rt
RUN: %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT_MUTATION
RUN: MULL="cxx_remove_void_call:%s:11:3:1" %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH_MUTATION

WITHOUT_MUTATION:NORMAL
WITH_MUTATION:MUTATED
*/
