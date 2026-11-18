// clang-format off
struct S {};
void operator<<(S, int);
bool operator<(S, S);
S &operator+=(S &, S);
S operator-(S);
S &operator++(S &);

template <typename T> T f(T a, T b) {
  T r = a << 1;
  r += a < b;
  r = -r;
  ++r;
  return r;
}

int main() {
  return f(1, 2) != -2;
}

// An overload visible at the template keeps 'a << 1' a CXXOperatorCallExpr in the pattern (#1226)

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/main.exe | %filecheck %s --dump-input=fail --match-full-lines
// CHECK:[info] Killed mutants (5/5):
// CHECK:[info] Mutation score: 100%
