// clang-format off

/**
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -g -O0 %s -o %s-ir.exe
RUN: %mull_runner -reporters=Elements -report-dir=%S -report-name=01_no_mutations %s-ir.exe | %filecheck %s --dump-input=fail
CHECK:[info] No mutants found. Mutation score: infinitely high
**/

int main() {
  return 0;
}
