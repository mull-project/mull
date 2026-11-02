// clang-format off

template <class T> T value() {
  T var{};
  return var;
}

int main() {
  int a{};
  int b = {};
  double c{};
  bool d{};
  int e{7};
  int f = {8};
  return a + b + c + d + value<int>() + e + f != 15;
}

/**
RUN: %clang_cxx %sysroot -O0 %pass_mull_ir_frontend -g -grecord-command-line %s -o %s-ir.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines --check-prefix=CHECK-MUTATE --implicit-check-not="CXXJunkDetector: mutation"

CHECK-MUTATE:{{.*}}7/7. Finished in {{.*}}
CHECK-MUTATE:[info] Applying filter: junk (threads: 1)
CHECK-MUTATE:{{.*}}[debug] CXXJunkDetector: mutation "Init Const": {{.*}}main.cpp:13:7 (end: 13:10)
CHECK-MUTATE:{{.*}}[debug] CXXJunkDetector: mutation "Init Const": {{.*}}main.cpp:14:7 (end: 14:13)

RUN: (unset TERM; %mull_runner -reporters=IDE -ide-reporter-show-killed %s-ir.exe 2>&1; test $? = 0) | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

CHECK:[info] Killed mutants (2/2):
CHECK-NEXT:{{^.*}}main.cpp:13:9: warning: Killed: Replaced 7 with 6 [cxx_init_const]{{$}}
CHECK-NEXT:  int e{7};
CHECK-NEXT:        ^
CHECK-NEXT:{{^.*}}main.cpp:14:12: warning: Killed: Replaced 8 with 9 [cxx_init_const]{{$}}
CHECK-NEXT:  int f = {8};
CHECK-NEXT:           ^
CHECK-NEXT:[info] Mutation score: 100%
*/
