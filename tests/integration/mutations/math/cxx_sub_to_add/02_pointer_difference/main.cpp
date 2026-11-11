// clang-format off
long distance(const char *p, const char *q) {
  return p - q;
}
int sub(int a, int b) {
  return a - b;
}
int main() {
  const char s[] = "abc";
  return distance(s + 2, s) != 2 || sub(3, 1) != 2;
}

// A pointer difference is not mutated, a sum of pointers is not a valid program

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (1/1):
// CHECK-NEXT:{{.*}}main.cpp:6:12: warning: Killed: Replaced - with + [cxx_sub_to_add]
// CHECK-NEXT:  return a - b;
// CHECK-NEXT:           ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -6,1 +6,1 @@
// CHECK-NEXT:-  return a - b;
// CHECK-NEXT:+  return a + b;
// CHECK-NEXT:[info] Mutation score: 100%
