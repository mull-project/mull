#include <assert.h>
#include <stdbool.h>

int main() {
  // A bool is negated: it is stored as a byte but read back through
  // `trunc i8 %v to i1`, so replacing it with 42 would read as false either
  // way and produce an equivalent mutant.
  bool flag = true;
  assert(flag);

  bool negative_flag = false;
  assert(!negative_flag);

  // Every other type keeps the 42 replacement, however bool-like it looks in
  // IR. `char small = 0` is the same `store i8 0` as `bool b = false`; what
  // differs is that it is read with `icmp`, not truncated to i1. Negating
  // these would turn mutants the asserts below kill into equivalent ones.
  char small = 0;
  assert(small <= 5);

  unsigned char unsigned_small = 1;
  assert(unsigned_small <= 5);

  signed char signed_small = 1;
  assert(signed_small <= 5);

  short shorter = 0;
  assert(shorter <= 5);

  int number = 1;
  assert(number <= 5);

  return 0;
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (14/14):
// CHECK:{{.*}}main.c:8:8: warning: Killed: Replaced with = 0; [cxx_assign_const]
// CHECK:{{.*}}main.c:8:8: warning: Killed: Replaced with 0 [cxx_init_const]
// CHECK:{{.*}}main.c:11:8: warning: Killed: Replaced with = 1; [cxx_assign_const]
// CHECK:{{.*}}main.c:11:8: warning: Killed: Replaced with 1 [cxx_init_const]
// CHECK:{{.*}}main.c:18:8: warning: Killed: Replaced with = 42; [cxx_assign_const]
// CHECK:{{.*}}main.c:18:8: warning: Killed: Replaced with 42 [cxx_init_const]
// CHECK:{{.*}}main.c:21:17: warning: Killed: Replaced with = 42; [cxx_assign_const]
// CHECK:{{.*}}main.c:21:17: warning: Killed: Replaced with 42 [cxx_init_const]
// CHECK:{{.*}}main.c:24:15: warning: Killed: Replaced with = 42; [cxx_assign_const]
// CHECK:{{.*}}main.c:24:15: warning: Killed: Replaced with 42 [cxx_init_const]
// CHECK:{{.*}}main.c:27:9: warning: Killed: Replaced with = 42; [cxx_assign_const]
// CHECK:{{.*}}main.c:27:9: warning: Killed: Replaced with 42 [cxx_init_const]
// CHECK:{{.*}}main.c:30:7: warning: Killed: Replaced with = 42; [cxx_assign_const]
// CHECK:{{.*}}main.c:30:7: warning: Killed: Replaced with 42 [cxx_init_const]
// CHECK:[info] All mutations have been killed
