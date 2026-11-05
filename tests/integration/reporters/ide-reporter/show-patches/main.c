int add(int a, int b) {
  return a + b;
}

int not_two(int x) {
  return x != 2;
}

int main() {
  add(15, 42);
  return not_two(2);
}

// clang-format off

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/Output/sandbox/main.c
// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %S/Output/sandbox/main.c -o %S/Output/main.exe
// RUN: cd %S/Output && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --check-prefixes=CHECK,SURVIVED
// RUN: cd %S/Output && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-patches -ide-reporter-show-killed -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --check-prefixes=KILLED,SURVIVED
// RUN: cd %S/Output && %mull_runner -allow-surviving -reporters=SQLite -report-name=patches %S/Output/main.exe
// RUN: cd %S/Output && %mull_reporter -allow-surviving -reporters=IDE -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/patches.sqlite | %filecheck %s --dump-input=fail --strict-whitespace --check-prefixes=CHECK,SURVIVED
// RUN: cd %S/Output && %mull_runner -allow-surviving -reporters=IDE %S/Output/main.exe | %filecheck %s --dump-input=fail --check-prefix=NO-PATCHES

// CHECK-NOT:Killed mutants
// CHECK-NOT:+{{.*}}x == 2;

// KILLED:{{.*}}Killed mutants (1/2):
// KILLED-NEXT:{{.*}}main.c:6:12: warning: Killed: Replaced != with == [cxx_ne_to_eq]
// KILLED-NEXT:  return x != 2;
// KILLED-NEXT:           ^
// KILLED-NEXT:--- a/sandbox/main.c 0
// KILLED-NEXT:+++ b/sandbox/main.c 0
// KILLED-NEXT:@@ -6,1 +6,1 @@
// KILLED-NEXT:-  return x != 2;
// KILLED-NEXT:+  return x == 2;

// SURVIVED:{{.*}}Survived mutants (1/2):
// SURVIVED-NEXT:{{.*}}main.c:2:12: warning: Survived: Replaced + with - [cxx_add_to_sub]
// SURVIVED-NEXT:  return a + b;
// SURVIVED-NEXT:           ^
// SURVIVED-NEXT:--- a/sandbox/main.c 0
// SURVIVED-NEXT:+++ b/sandbox/main.c 0
// SURVIVED-NEXT:@@ -2,1 +2,1 @@
// SURVIVED-NEXT:-  return a + b;
// SURVIVED-NEXT:+  return a - b;
// SURVIVED-NEXT:{{.*}}Mutation score: 50%

// NO-PATCHES:{{.*}}main.c:2:12: warning: Survived: Replaced + with - [cxx_add_to_sub]
// NO-PATCHES-NOT:--- a/
