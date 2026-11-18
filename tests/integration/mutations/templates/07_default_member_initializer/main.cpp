// clang-format off
struct S {};
S operator+(S, S);

int base = 1;

struct Plain {
  int value = base + 1;
};

template <typename T> struct Boxed {
  T value = T(base) + 2;
  Boxed() {}
};

template <typename T> struct Implicit {
  T value = T(base) + 3;
};

int main() {
  Plain p;
  Boxed<int> b;
  Implicit<int> i;
  return p.value != 2 || b.value != 3 || i.value != 4;
}

// Constructors run the default member initializers, which are written outside of them

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/main.exe | %filecheck %s --dump-input=fail --match-full-lines
// CHECK:[info] Killed mutants (3/3):
// CHECK-NEXT:{{.*}}main.cpp:8:20: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:12:21: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:17:21: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:[info] Mutation score: 100%
