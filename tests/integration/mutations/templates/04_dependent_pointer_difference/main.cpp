// clang-format off
template <typename P> long d(P p, P q) {
  return p - q;
}
int main() {
  int a[4];
  return d(a + 3, a) != 3;
}

// 'p + q' of pointers is not a valid program, P is only known to be a pointer in the instantiation

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/main.exe | %filecheck %s --dump-input=fail --match-full-lines
// CHECK:[info] No mutants found. Mutation score: infinitely high
