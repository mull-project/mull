static int global_state = 0;

void void_function() {
  global_state = 1;
}

int test_func_with_a_void_function_inside() {
  void_function();
  if (global_state == 0) {
    return 1;
  }
  return 0;
}

int main() {
  return test_func_with_a_void_function_inside();
}

// clang-format off

// RUN: cd / && %clang_cc -fembed-bitcode -g -O0 %s -o %s.exe
// RUN: cd %CURRENT_DIR
// RUN: unset TERM; %MULL_EXEC -linker=%clang_cc -test-framework CustomTest -mutators=remove_void_function_mutator -ide-reporter-show-killed -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}8:3: warning: Killed: Remove Void Call: removed void_function [remove_void_function_mutator]
// CHECK:  void_function();
// CHECK:  ^
// CHECK: [info] All mutations have been killed
