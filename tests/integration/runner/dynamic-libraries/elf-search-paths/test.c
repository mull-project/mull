int sum(int, int);

#ifdef SHARED_LIB
int sum(int a, int b) {
  return a + b;
}
#endif

#ifdef TEST_BIN
int main() {
  int r = sum(2, 5) == 7;
  return !r;
}
#endif

// clang-format off

/*
Libraries without RPATH/RUNPATH are resolved via -ld-search-path or LD_LIBRARY_PATH, and reported otherwise

REQUIRES: LINUX
RUN: rm -rf %t && mkdir -p %t/lib
RUN: %clang_cc %sysroot -DSHARED_LIB -fPIC -shared %s %pass_mull_ir_frontend -g -grecord-command-line -Wl,-soname,libsum.so -o %t/lib/libsum.so
RUN: %clang_cc %sysroot -DTEST_BIN %s -L%t/lib -lsum -o %t/test-ir.exe

RUN: cd /; unset TERM; %mull_runner %t/test-ir.exe -test-program /usr/bin/env -- LD_LIBRARY_PATH=%t/lib %t/test-ir.exe 2>&1 | %filecheck %s --dump-input=fail --match-full-lines --check-prefix=NOT-FOUND
NOT-FOUND:[warning] Could not find dynamic library: libsum.so
NOT-FOUND:[info] No mutants found. Mutation score: infinitely high

RUN: cd /; unset TERM; %mull_runner -ide-reporter-show-killed -ld-search-path=%t/lib %t/test-ir.exe -test-program /usr/bin/env -- LD_LIBRARY_PATH=%t/lib %t/test-ir.exe 2>&1 | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not="Could not find dynamic library"
RUN: cd /; unset TERM; env LD_LIBRARY_PATH=%t/lib %mull_runner -ide-reporter-show-killed %t/test-ir.exe 2>&1 | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not="Could not find dynamic library"
CHECK:{{.*}}test.c:5:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
CHECK:  return a + b;
CHECK:           ^
*/
