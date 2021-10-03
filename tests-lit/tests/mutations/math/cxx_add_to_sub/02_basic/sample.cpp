// clang-format off

/**
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -reporters=IDE %s.exe | %filecheck %s --dump-input=fail
CHECK:[info] Running mutants (threads: 1)
CHECK:{{^       \[################################\] 1/1\. Finished .*}}
CHECK:[info] All mutations have been killed
CHECK:[info] Mutation score: 100%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}
