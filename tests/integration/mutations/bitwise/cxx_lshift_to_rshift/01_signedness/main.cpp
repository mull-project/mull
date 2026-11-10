// clang-format off
#include <stdio.h>
int main(int argc, char **) {
  int x = -3 * argc;
  unsigned u = 0x80000000u * argc;
  printf("%d\n", x << 1);
  printf("%u\n", u << 1);
}

// '>>' is arithmetic for a signed value and logical for an unsigned one

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: env "cxx_lshift_to_rshift:%S/Output/sandbox/main.cpp:6:20:6:22=1" %S/Output/main.exe | %filecheck %s --match-full-lines --check-prefix=SIGNED
// RUN: env "cxx_lshift_to_rshift:%S/Output/sandbox/main.cpp:7:20:7:22=1" %S/Output/main.exe | %filecheck %s --match-full-lines --check-prefix=UNSIGNED
// SIGNED:-2
// SIGNED-NEXT:0
// UNSIGNED:-6
// UNSIGNED-NEXT:1073741824
