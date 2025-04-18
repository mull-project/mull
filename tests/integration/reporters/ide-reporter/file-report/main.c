int add(int a, int b) {
  return a + b;
}

int main() {
  add(15, 42);
  return 0;
}

// clang-format off

// RUN: mkdir -p Output
// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: %mull_runner -allow-surviving -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// RUN: %mull_runner -allow-surviving -reporters=IDE --report-dir=Output --report-name=ide %s-ir.exe
// RUN: cat Output/ide.txt | %filecheck %s --dump-input=fail
// CHECK:{{.*}}Survived mutants (1/1):
// CHECK:{{.*}}main.c:2:12: warning: Survived: Replaced + with - [cxx_add_to_sub]
// CHECK:  return a + b;
// CHECK:           ^
// CHECK:{{.*}}Mutation score: 0%
