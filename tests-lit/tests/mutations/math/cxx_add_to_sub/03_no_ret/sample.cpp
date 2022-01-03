// clang-format off

void sum_noop(int a, int b) {
  a + b;
}

int main() {
  sum_noop(-2, 2);
  return 0;
}

/**
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -g -O0 %s -o %s-ir.exe
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %filecheck %s --dump-input=fail
RUN: unset TERM; %mull_runner -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
CHECK-NOT:[error]
**/
