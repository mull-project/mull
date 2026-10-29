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
Libraries are resolved via DT_RUNPATH and DT_RPATH with $ORIGIN, without -ld-search-path

REQUIRES: LINUX
RUN: rm -rf %t && mkdir -p %t/lib
RUN: %clang_cc %sysroot -DSHARED_LIB -fPIC -shared %s %pass_mull_ir_frontend -g -grecord-command-line -Wl,-soname,libsum.so -o %t/lib/libsum.so
RUN: %clang_cc %sysroot -DTEST_BIN %s -Wl,--enable-new-dtags -Wl,-rpath,'$ORIGIN/lib' -L%t/lib -lsum -o %t/test-runpath.exe
RUN: %clang_cc %sysroot -DTEST_BIN %s -Wl,--disable-new-dtags -Wl,-rpath,'$ORIGIN/lib' -L%t/lib -lsum -o %t/test-rpath.exe

RUN: cd /; unset TERM; %mull_runner -ide-reporter-show-killed %t/test-runpath.exe 2>&1 | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not="Could not find dynamic library"
RUN: cd /; unset TERM; %mull_runner -ide-reporter-show-killed %t/test-rpath.exe 2>&1 | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not="Could not find dynamic library"
CHECK:{{.*}}test.c:5:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
CHECK:  return a + b;
CHECK:           ^
*/
