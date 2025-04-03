// clang-format off

#include "header.h"
#include "header2.h"

// mull-off
int ignored_block(int a, int b) {
  return a + b;
}
// mull-on

int ignore_same_line(int a, int b) {
  return a + b; // mull-ignore
}

int ignore_next_line(int a, int b) {
  // mull-ignore-next
  return a + b;
}

int ignore_same_line_add(int a, int b, int c) {
  return a + b - c; // mull-ignore: cxx_add_to_sub
}

int ignore_next_line_add(int a, int b, int c) {
  // mull-ignore-next: cxx_add_to_sub
  return a + b - c;
}

int main(int argc, char **argv) {
  ignored_block(2, 42);
  add_from_header(15, 22);
  sub_from_header(15, 22);
  add_from_header2(15, 22);
  sub_from_header2(15, 22);
  ignore_same_line(2, 23);
  ignore_next_line(2, 23);
  ignore_same_line_add(2, 23, 15);
  ignore_next_line_add(2, 23, 55);
}

// RUN: %clang_cc %sysroot %pass_mull_ir_frontend -g %s -o %s.exe

// RUN: unset TERM; %mull_runner --allow-surviving --ide-reporter-show-killed %s.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines

// CHECK:[info] Survived mutants (4/4):
// CHECK-NEXT:{{.*}}/header.h:7:12: warning: Survived: Replaced - with + [cxx_sub_to_add]
// CHECK-NEXT:  return a - b;
// CHECK-NEXT:           ^
// CHECK-NEXT:{{.*}}/header2.h:7:12: warning: Survived: Replaced - with + [cxx_sub_to_add]
// CHECK-NEXT:  return a - b;
// CHECK-NEXT:           ^
// CHECK-NEXT:{{.*}}/main.c:22:16: warning: Survived: Replaced - with + [cxx_sub_to_add]
// CHECK-NEXT:  return a + b - c; // mull-ignore: cxx_add_to_sub
// CHECK-NEXT:               ^
// CHECK-NEXT:{{.*}}/main.c:27:16: warning: Survived: Replaced - with + [cxx_sub_to_add]
// CHECK-NEXT:  return a + b - c;
// CHECK-NEXT:               ^
// CHECK-NEXT:[info] Mutation score: 0%
