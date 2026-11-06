// clang-format off
#include <stdio.h>
int main(int argc, char **) {
  int x = -3 * argc;
  printf("%d\n", x << 1);
}

// Bug: '<<' is mutated into a logical shift, '>>' on a signed value is arithmetic
// XFAIL: *

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: env "cxx_lshift_to_rshift:%S/Output/sandbox/main.cpp:5:20:5:22=1" %S/Output/main.exe | %filecheck %s --match-full-lines
// CHECK:-2
