// clang-format off
#define CHECK(cond) ((cond) ? 0 : 1)
int main(int argc, char **) {
  return CHECK(argc + 1 == 2);
}

// Bug: mutants in macro arguments end at 0:0
// XFAIL: *

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (2/2):
// CHECK-NEXT:{{.*}}main.cpp:4:21: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK-NEXT:  return CHECK(argc + 1 == 2);
// CHECK-NEXT:                    ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -4,1 +4,1 @@
// CHECK-NEXT:-  return CHECK(argc + 1 == 2);
// CHECK-NEXT:+  return CHECK(argc - 1 == 2);
// CHECK-NEXT:{{.*}}main.cpp:4:25: warning: Killed: Replaced == with != [cxx_eq_to_ne]
// CHECK-NEXT:  return CHECK(argc + 1 == 2);
// CHECK-NEXT:                        ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -4,1 +4,1 @@
// CHECK-NEXT:-  return CHECK(argc + 1 == 2);
// CHECK-NEXT:+  return CHECK(argc + 1 != 2);
// CHECK-NEXT:[info] Mutation score: 100%
