// clang-format off

/**
RUN: %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
RUN: unset TERM; %mull_runner -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
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
