// clang-format off

int sum(int a, int b) {
  return a + b;
}

int main() {
  return sum(-2, 2);
}

/**
RUN: %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s.exe
RUN: MULL_CONFIG=%S/mull-group.yml %clang_cxx %sysroot %pass_mull_ir_frontend -g %s -o %s-group.exe
RUN: unset TERM; %mull_runner -reporters=IDE %s.exe | %filecheck %s --dump-input=fail
RUN: unset TERM; %mull_runner -reporters=IDE %s-group.exe | %filecheck %s --dump-input=fail
CHECK:[info] No mutants found. Mutation score: infinitely high
**/
