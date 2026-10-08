int helper(void);

#ifdef SHARED_LIB
int helper(void) {
  return 0;
}
#endif

#ifdef TEST_BIN
int never_called(int a, int b) {
  return a + b;
}

int main() {
  return helper();
}
#endif

// clang-format off

/*
A dynamic dependency that has no coverage mapping at all (like the system libc++) must not prevent
the coverage report of the executable from being generated.

RUN: %clang_cc %sysroot -DSHARED_LIB -fPIC -shared %s -gno-record-command-line -g -o %S/libnocov.lib
RUN: cd %S; %clang_cc %sysroot -DTEST_BIN ./main.c %pass_mull_ir_frontend -g -grecord-command-line -fprofile-instr-generate -fcoverage-mapping ./libnocov.lib -o ./main-cov.exe

RUN: cd %S; unset TERM; %mull_runner --allow-surviving -ld-search-path=%S -include-not-covered ./main-cov.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK-NOT:{{.*}}Cannot generate coverage report{{.*}}
CHECK:[info] Not Covered mutants (1/1):
CHECK:{{^.*}}main.c:11:12: warning: Not Covered: Replaced + with - [cxx_add_to_sub]
CHECK:[info] Mutation score: 0%
*/
