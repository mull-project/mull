// clang-format off
#include <stdio.h>
template <typename T> T shl1(T x) {
  return x << 1;
}
int main(int argc, char **) {
  int s = shl1(-4 * argc);
  unsigned u = shl1(0x80000000u * argc);
  printf("%d\n%u\n", s, u);
  return s != -8 || u != 0;
}

// Both instantiations share the mutant's location, each one keeps the '>>' of its own signedness

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: (env "cxx_lshift_to_rshift:%S/Output/sandbox/main.cpp:4:12:4:14=1" %S/Output/main.exe || true) | %filecheck %s --match-full-lines
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/main.exe | %filecheck %s --dump-input=fail --match-full-lines --check-prefix=RUNNER
// CHECK:-2
// CHECK-NEXT:1073741824
// RUNNER:[info] Killed mutants (1/1):
// RUNNER-NEXT:{{.*}}main.cpp:4:12: warning: Killed: Replaced << with >> [cxx_lshift_to_rshift]
