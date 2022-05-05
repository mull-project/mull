// clang-format off

int main() {
  int a = 1;
  a = a && a && a;
  int b = 2;
  b = b || b || b;
  return 0;
}

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s.exe
// RUN: unset TERM; %mull_runner -reporters=IDE %s.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Survived mutants (3/3):
// CHECK-NEXT:{{.*}}main.c:5:9: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK-NEXT:  a = a && a && a;
// CHECK-NEXT:        ^
// CHECK-NEXT:{{.*}}main.c:5:14: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK-NEXT:  a = a && a && a;
// CHECK-NEXT:             ^
// CHECK-NEXT:{{.*}}main.c:7:9: warning: Survived: Replaced || with && [cxx_logical_or_to_and]
// CHECK-NEXT:  b = b || b || b;
// CHECK-NEXT:        ^
// CHECK-NEXT:[info] Mutation score: 0%
