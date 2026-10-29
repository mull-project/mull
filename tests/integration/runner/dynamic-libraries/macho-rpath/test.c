int inner_sum(int, int);
int outer_sum(int, int);

#ifdef INNER_LIB
int inner_sum(int a, int b) {
  return a + b;
}
#endif

#ifdef OUTER_LIB
int outer_sum(int a, int b) {
  return inner_sum(a, b);
}
#endif

#ifdef TEST_BIN
int main() {
  int r = outer_sum(2, 5) == 7;
  return !r;
}
#endif

// clang-format off

/*
Mutants are found in a transitive dependency without -ld-search-path:
test-ir.exe -> @rpath/libouter.dylib (no mutants) -> @loader_path/libinner.dylib (mutants)

REQUIRES: MACOS
RUN: rm -rf %t && mkdir -p %t/lib
RUN: %clang_cc %sysroot -DINNER_LIB -dynamiclib %s %pass_mull_ir_frontend -g -grecord-command-line -install_name @loader_path/libinner.dylib -o %t/lib/libinner.dylib
RUN: %clang_cc %sysroot -DOUTER_LIB -dynamiclib %s -install_name @rpath/libouter.dylib -L%t/lib -linner -o %t/lib/libouter.dylib
RUN: %clang_cc %sysroot -DTEST_BIN %s -Wl,-rpath,@executable_path/lib -L%t/lib -louter -o %t/test-ir.exe

RUN: cd /; unset TERM; %mull_runner -ide-reporter-show-killed %t/test-ir.exe 2>&1 | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not="Could not find dynamic library"
CHECK:{{.*}}test.c:6:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
CHECK:  return a + b;
CHECK:           ^
*/
