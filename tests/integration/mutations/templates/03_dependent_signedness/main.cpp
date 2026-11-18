// clang-format off
#include <stdio.h>
template <typename T> T shl1(T x) {
  return x << 1;
}
int main(int argc, char **) {
  printf("%u\n", shl1(0x80000000u * argc));
}

// '>>' on an unsigned T is logical, the dependent pattern type must not pick 'ashr'

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: env "cxx_lshift_to_rshift:%S/Output/sandbox/main.cpp:4:12:4:14=1" %S/Output/main.exe | %filecheck %s --match-full-lines
// CHECK:1073741824
