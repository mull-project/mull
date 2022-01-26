// clang-format off

/**
RUN: cd %S && %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
RUN: (unset TERM; %mull_runner -ide-reporter-show-killed %s-ir.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
CHECK:[info] Killed mutants (1/2):
CHECK:{{^.*}}sample.cpp:14:11: warning: Killed: Replaced >= with < [cxx_ge_to_lt]{{$}}
CHECK:[info] Survived mutants (1/2):
CHECK:{{^.*}}sample.cpp:14:11: warning: Survived: Replaced >= with > [cxx_ge_to_gt]{{$}}
CHECK:[info] Mutation score: 50%
**/

bool valid_age(int age) {
  if (age >= 21) {
    return true;
  }
  return false;
}

int main() {
  bool test1 = valid_age(25) == true;
  if (!test1) {
    /// test failed
    return 1;
  }

  bool test2 = valid_age(20) == false;
  if (!test2) {
    /// test failed
    return 1;
  }

  /// success
  return 0;
}
