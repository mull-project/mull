int neg1(int a) {
  return !a; // killed
}

int neg2(int a) {
  return !a; // survived
}

int main() {
  neg2(14);
  return neg1(42);
}

// clang-format off

/**
RUN: %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
RUN: cd / && %clang_cxx %sysroot -fembed-bitcode -g -O0 %s -o %s.exe
RUN: cd %CURRENT_DIR
RUN: unset TERM; %mull_cxx -keep-executable -output=%s.mutated.exe -linker=%clang_cxx -linker-flags="%sysroot" -workers=1 -mutators=cxx_remove_negation --ide-reporter-show-killed -reporters=IDE %s.exe | %filecheck %s --dump-input=fail
RUN: unset TERM; %mull_runner -workers=1 --ide-reporter-show-killed -reporters=IDE %s.mutated.exe | %filecheck %s --dump-input=fail
RUN: unset TERM; %mull_runner -workers=1 --ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
CHECK:[info] Running mutants (threads: 1)
CHECK:{{^       \[################################\] 2/2\. Finished .*}}
CHECK:[info] Killed mutants (1/2):
CHECK:{{^.*}}main.cpp:2:10: warning: Killed: Replaced !a with a [cxx_remove_negation]{{$}}
CHECK:[info] Survived mutants (1/2):
CHECK:{{^.*}}main.cpp:6:10: warning: Survived: Replaced !a with a [cxx_remove_negation]{{$}}
CHECK:[info] Mutation score: 50%
CHECK:[info] Total execution time: {{.*}}
CHECK-EMPTY:
**/
