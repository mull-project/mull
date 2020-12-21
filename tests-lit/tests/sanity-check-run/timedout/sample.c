extern unsigned int sleep(unsigned int);
int main() {
  sleep(1);
  return 1;
}

// clang-format off

// RUN: cd / && %clang_cc -fembed-bitcode %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: (unset TERM; %MULL_EXEC -linker=%clang_cc -timeout=100 -workers=1 -test-framework CustomTest %s.exe 2>&1; test $? = 0) | %FILECHECK_EXEC %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[warning] Original test failed
// CHECK-NEXT:test: main
// CHECK-NEXT:status: Timedout
// CHECK-NEXT:stdout: ''
// CHECK-NEXT:stderr: ''
