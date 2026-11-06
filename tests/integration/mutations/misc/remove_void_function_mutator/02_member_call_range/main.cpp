// clang-format off
struct vec {
  int size = 0;
  void resize(int n) {
    size = n;
  }
};
int main() {
  vec v;
  v.resize(2);
  return v.size != 2;
}

// Bug: a member call mutant covers only the member name
// XFAIL: *

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (1/1):
// CHECK-NEXT:{{.*}}main.cpp:10:3: warning: Killed: Replaced v.resize(2) with  [cxx_remove_void_call]
// CHECK-NEXT:  v.resize(2);
// CHECK-NEXT:  ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -10,1 +10,1 @@
// CHECK-NEXT:-  v.resize(2);
// CHECK-NEXT:+  ;
// CHECK-NEXT:[info] Mutation score: 100%
