#include <stdio.h>

int main() {
  fprintf(stderr, "failed stderr");
  fflush(stderr);
  fprintf(stdout, "failed stdout");
  fflush(stdout);
  return 1;
}

// clang-format off
// RUN: cd / && %clang_cc %sysroot %TEST_CXX_FLAGS %pass_mull_ir_frontend %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: (unset TERM; %mull_runner -workers=1 %s.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[warning] Original test failed
// CHECK-NEXT:status: Failed
// CHECK-NEXT:stdout: 'failed stdout'
// CHECK-NEXT:stderr: 'failed stderr'
