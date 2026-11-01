int add(int a, int b) {
  return a + b;
}

int main() {
  return add(2, 3) == 5 ? 0 : 1;
}

// clang-format off

/**
Junk detection re-parses the unit from the path recorded in the debug info. When that path does not
exist the mutant is junk, the compiler must not crash.

RUN: cd %S && env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot %pass_mull_ir_frontend -g -ffile-prefix-map=%S=/nonexistent -c main.cpp -o %t-prefix-map.o 2>&1 | %filecheck %s --dump-input=fail
RUN: cd %S && cat main.cpp | env MULL_CONFIG=%S/mull.yml %clang_cxx %sysroot %pass_mull_ir_frontend -g -x c++ -c - -o %t-stdin.o 2>&1 | %filecheck %s --dump-input=fail

CHECK-NOT:{{.*}}Stack dump{{.*}}
CHECK:{{.*}}source location does not exist{{.*}}
CHECK-NOT:{{.*}}Stack dump{{.*}}
**/
