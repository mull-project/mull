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

RUN: %clang_cc %sysroot -DSHARED_LIB -fPIC -shared %s %pass_mull_ir_frontend -g -o %S/shared-ir.lib
RUN: cd %S; %clang_cc %sysroot -DTEST_BIN ./test.c %S/shared-ir.lib -o ./test-ir.exe

RUN: cd /; unset TERM; %mull_runner -ld-search-path=%S -ide-reporter-show-killed %S/test-ir.exe | %filecheck %s --dump-input=fail --match-full-lines --check-prefix=CHECK
CHECK:{{.*}}test.c:5:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
CHECK:  return a + b;
CHECK:           ^
*/
