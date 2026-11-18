// clang-format off
struct S {};
S operator+(S, S);

int base = 1;

int five() {
  return 5;
}

struct Pair {
  int a, b;
};

int plain(int x = base + 1) {
  return x;
}

template <typename T> T templated(T x = T(base) + 2) {
  return x;
}

Pair make(int x = five()) {
  return { x, x };
}

int main() {
  Pair p = make();
  return plain() != 2 || templated<int>() != 3 || p.a != 5;
}

// Default arguments are emitted in the caller at the call's location, there is no source for their mutants

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/main.exe | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not=cxx_add_to_sub --implicit-check-not='Replaced make'
// CHECK:[info] Killed mutants (4/4):
// CHECK-NEXT:{{.*}}main.cpp:29:10: warning: Killed: Replaced plain() with 42 [cxx_replace_scalar_call_nz]
// CHECK:{{.*}}main.cpp:29:10: warning: Killed: Replaced plain() with 0 [cxx_replace_scalar_call_zr]
// CHECK:{{.*}}main.cpp:29:26: warning: Killed: Replaced templated<int>() with 42 [cxx_replace_scalar_call_nz]
// CHECK:{{.*}}main.cpp:29:26: warning: Killed: Replaced templated<int>() with 0 [cxx_replace_scalar_call_zr]
// CHECK:[info] Mutation score: 100%
