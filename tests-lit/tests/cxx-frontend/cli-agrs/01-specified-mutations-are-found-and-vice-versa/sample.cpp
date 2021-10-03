extern "C" {
extern int printf(const char *, ...);
}

int sum(int a, int b) {
  return a + b;
}

int main() {
  if (sum(-2, 2) == 0) {
    printf("NORMAL\n");
  } else {
    printf("MUTATED\n");
  }
  return 0;
}

// clang-format off

/**
RUN: %clang_cxx %sysroot -fplugin=%mull_frontend_cxx -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_add_to_sub %s -o %s.exe
RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=RIGHT_MUTATION_WITHOUT
RUN: env "cxx_add_to_sub:%s:6:12"=1 %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=RIGHT_MUTATION_WITH

RIGHT_MUTATION_WITHOUT:NORMAL
RIGHT_MUTATION_WITH:MUTATED

RUN: %clang_cxx %sysroot -fplugin=%mull_frontend_cxx -Xclang -plugin-arg-mull-cxx-frontend -Xclang mutators=cxx_logical_or_to_and %s -o %s.exe
RUN: %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WRONG_MUTATION_WITHOUT
RUN: env "cxx_add_to_sub:%s:6:12"=1 %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=WRONG_MUTATION_WITH

WRONG_MUTATION_WITHOUT:NORMAL
WRONG_MUTATION_WITH:NORMAL
*/
