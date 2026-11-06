// clang-format off
struct buffer {
  int size = 0;
  char data[8];
  __attribute__((noinline)) void push_back(const char &c) {
    data[size++] = c;
  }
};
int main() {
  buffer b;
  b.push_back('a');
  return b.size != 1;
}

// At -O1+ cxx_remove_void_call removes the call, not the llvm.lifetime.end of its temporary

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O3 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed -ide-reporter-show-patches -report-patch-base=%S/Output %S/Output/main.exe | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (1/1):
// CHECK-NEXT:{{.*}}main.cpp:11:3: warning: Killed: Replaced b.push_back('a') with  [cxx_remove_void_call]
// CHECK-NEXT:  b.push_back('a');
// CHECK-NEXT:  ^
// CHECK-NEXT:--- a/sandbox/main.cpp
// CHECK-NEXT:+++ b/sandbox/main.cpp
// CHECK-NEXT:@@ -11,1 +11,1 @@
// CHECK-NEXT:-  b.push_back('a');
// CHECK-NEXT:+  ;
// CHECK-NEXT:[info] Mutation score: 100%
