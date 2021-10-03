extern void exit(int);
int main() {
  exit(142);
  return 1;
}

// clang-format off

// This status may be abandoned eventually
// XFAIL: *

// RUN: cd / && %clang_cc %sysroot -fembed-bitcode %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: (unset TERM; %mull_cxx -linker=%clang_cc -linker-flags="%sysroot" -timeout=100 -workers=1 %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[warning] Original test failed
// CHECK-NEXT:test: main
// CHECK-NEXT:status: AbnormalExit
// CHECK-NEXT:stdout: ''
// CHECK-NEXT:stderr: ''
