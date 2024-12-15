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

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}8:3: warning: Killed: Removed the call to the function [cxx_remove_void_call]
// CHECK:  void_function();
// CHECK:  ^
// CHECK: [info] All mutations have been killed
