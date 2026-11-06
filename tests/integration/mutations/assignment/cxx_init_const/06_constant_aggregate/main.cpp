// clang-format off
#include <time.h>
constexpr tm make_tm(int year) {
  auto t = tm();
  t.tm_year = year - 1900;
  return t;
}
int main() {
  auto t = make_tm(1975);
  int year = 75;
  return t.tm_year != year;
}

// Bug: cxx_init_const mutates a field of a constant struct initializer
// XFAIL: *

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (1/1):
// CHECK-NEXT:{{.*}}main.cpp:10:14: warning: Killed: Replaced 75 with 74 [cxx_init_const]
// CHECK-NEXT:  int year = 75;
// CHECK-NEXT:             ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -10,1 +10,1 @@
// CHECK-NEXT:-  int year = 75;
// CHECK-NEXT:+  int year = 74;
// CHECK-NEXT:[info] Mutation score: 100%
