// clang-format off
#define CHECK(cond) ((cond) ? 0 : 1)
#define ID(x) x
#define ONE 1
#define INC(x) ((x) + 1)
int main(int argc, char **) {
  int a = CHECK(argc + 1 == 2);
  int b = ID(argc + 2);
  int c = ID(ID(argc + 3));
  int d = ID(argc + ONE);
  int e = INC(argc);
  return (a ^ b ^ c ^ d ^ e) != 7;
}

// Mutants in macro arguments are placed where they are written, the one in the body of INC is junk
// REQUIRES: llvm_22_or_higher

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (5/5):
// CHECK-NEXT:{{.*}}main.cpp:7:22: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK-NEXT:  int a = CHECK(argc + 1 == 2);
// CHECK-NEXT:                     ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -7,1 +7,1 @@
// CHECK-NEXT:-  int a = CHECK(argc + 1 == 2);
// CHECK-NEXT:+  int a = CHECK(argc - 1 == 2);
// CHECK-NEXT:{{.*}}main.cpp:7:26: warning: Killed: Replaced == with != [cxx_eq_to_ne]
// CHECK-NEXT:  int a = CHECK(argc + 1 == 2);
// CHECK-NEXT:                         ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -7,1 +7,1 @@
// CHECK-NEXT:-  int a = CHECK(argc + 1 == 2);
// CHECK-NEXT:+  int a = CHECK(argc + 1 != 2);
// CHECK-NEXT:{{.*}}main.cpp:8:19: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK-NEXT:  int b = ID(argc + 2);
// CHECK-NEXT:                  ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -8,1 +8,1 @@
// CHECK-NEXT:-  int b = ID(argc + 2);
// CHECK-NEXT:+  int b = ID(argc - 2);
// CHECK-NEXT:{{.*}}main.cpp:9:22: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK-NEXT:  int c = ID(ID(argc + 3));
// CHECK-NEXT:                     ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -9,1 +9,1 @@
// CHECK-NEXT:-  int c = ID(ID(argc + 3));
// CHECK-NEXT:+  int c = ID(ID(argc - 3));
// CHECK-NEXT:{{.*}}main.cpp:10:19: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK-NEXT:  int d = ID(argc + ONE);
// CHECK-NEXT:                  ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -10,1 +10,1 @@
// CHECK-NEXT:-  int d = ID(argc + ONE);
// CHECK-NEXT:+  int d = ID(argc - ONE);
// CHECK-NEXT:[info] Mutation score: 100%
