#include <stdio.h>

int main() {
  fprintf(stderr, "failed stderr");
  fflush(stderr);
  fprintf(stdout, "failed stdout");
  fflush(stdout);
  return 1;
}

// clang-format off

// RUN: cd / && %clang_cc %TEST_CXX_FLAGS -fembed-bitcode %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: (unset TERM; %MULL_EXEC -linker=%clang_cc -workers=1 %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[warning] Original test failed
// CHECK-NEXT:test: main
// CHECK-NEXT:status: Failed
// CHECK-NEXT:stdout: 'failed stdout'
// CHECK-NEXT:stderr: 'failed stderr'
