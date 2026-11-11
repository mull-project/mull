// clang-format off
#include <stdio.h>
int main(int argc, char **) {
  int x = -8 * argc;
  unsigned u = 4000000000u * argc;
  printf("%d\n", x * 2);
  printf("%u\n", u * 2);
}

// '/' is a signed division for signed values and an unsigned one for unsigned values

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: env "cxx_mul_to_div:%S/Output/sandbox/main.cpp:6:20:6:21=1" %S/Output/main.exe | %filecheck %s --match-full-lines --check-prefix=SIGNED
// RUN: env "cxx_mul_to_div:%S/Output/sandbox/main.cpp:7:20:7:21=1" %S/Output/main.exe | %filecheck %s --match-full-lines --check-prefix=UNSIGNED
// SIGNED:-4
// SIGNED-NEXT:3705032704
// UNSIGNED:-16
// UNSIGNED-NEXT:2000000000
