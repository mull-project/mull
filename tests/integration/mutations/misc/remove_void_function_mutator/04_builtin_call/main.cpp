// clang-format off
int main() {
  char source[8] = "mull";
  char target[8] = {};
  __builtin_memcpy_inline(target, source, 8);
  return target[0] != 'm';
}

// A builtin call compiled into an intrinsic, here `llvm.memcpy.inline`, is removed like any other call

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (1/1):
// CHECK-NEXT:{{.*}}main.cpp:5:3: warning: Killed: Replaced __builtin_memcpy_inline(target, source, 8) with  [cxx_remove_void_call]
// CHECK-NEXT:  __builtin_memcpy_inline(target, source, 8);
// CHECK-NEXT:  ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -5,1 +5,1 @@
// CHECK-NEXT:-  __builtin_memcpy_inline(target, source, 8);
// CHECK-NEXT:+  ;
// CHECK-NEXT:[info] Mutation score: 100%
