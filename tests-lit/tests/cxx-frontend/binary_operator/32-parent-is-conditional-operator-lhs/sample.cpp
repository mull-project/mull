extern "C" {
extern int printf(const char *, ...);
}

bool orFunc(bool a, bool b) {
  return (true ? a || b : false);
}

int main() {
  if (orFunc(false, true)) {
    printf("NORMAL\n");
  } else {
    printf("MUTATED\n");
  }
  return 0;
}

// clang-format off

/**
RUN: %CLANG_EXEC -Xclang -load -Xclang %mull_frontend_cxx -Xclang -add-plugin -Xclang mull-cxx-frontend -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_logical_or_to_and %s -o %s.exe %mull_rt -Wl,-rpath,%mull_path_to_rt
RUN: %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITHOUT_MUTATION
RUN: MULL="cxx_logical_or_to_and:%s:6:20:1" %s.exe | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WITH_MUTATION

WITHOUT_MUTATION:NORMAL
WITH_MUTATION:MUTATED
*/
