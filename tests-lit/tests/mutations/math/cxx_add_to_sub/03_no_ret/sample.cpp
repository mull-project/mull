// clang-format off

void sum_noop(int a, int b) {
  a + b;
}

int main() {
  sum_noop(-2, 2);
  return 0;
}

/**
RUN: cd / && %CLANG_EXEC -fembed-bitcode -g -O0 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: unset TERM; %MULL_EXEC -linker=%clang_cxx -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %FILECHECK_EXEC %s --dump-input=fail
CHECK-NOT:[error]
**/
