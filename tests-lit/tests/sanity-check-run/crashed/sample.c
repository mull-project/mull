extern void abort(void);
int main() {
  abort();
  return 1;
}

// clang-format off

// I cannot determine yet whether a process has signaled or not
// https://github.com/DaanDeMeyer/reproc/issues/49
// XFAIL: *

// RUN: cd / && %clang_cc %sysroot -fembed-bitcode %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: (unset TERM; %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" -workers=1 %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[warning] Original test failed
// CHECK-NEXT:test: main
// CHECK-NEXT:status: Crashed
// CHECK-NEXT:stdout: ''
// CHECK-NEXT:stderr: ''
