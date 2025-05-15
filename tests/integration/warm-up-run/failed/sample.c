#include <stdio.h>

int main() {
  fprintf(stderr, "failed stderr");
  fflush(stderr);
  fprintf(stdout, "failed stdout");
  fflush(stdout);
  return 1;
}

// clang-format off
// RUN: cd / && %clang_cc %sysroot %pass_mull_ir_frontend %s -o %s.exe
// RUN: (unset TERM; %mull_runner -workers=1 %s.exe 2>&1; test $? = 1) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[error] Original test failed
// CHECK-NEXT:status: Failed
// CHECK-NEXT:stdout: 'failed stdout'
// CHECK-NEXT:stderr: 'failed stderr'
