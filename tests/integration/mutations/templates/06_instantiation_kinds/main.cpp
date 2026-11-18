// clang-format off
struct S {};
S operator+(S, S);

int destroyed = 0;

template <typename T> struct Box {
  T value;
  Box(T a, T b) : value(0) {
    value = a + b;
  }
  ~Box() {
    destroyed = destroyed + 1;
  }
  T add(T other) const {
    return value + other;
  }
};

template <typename T> T viaLambda(T a, T b) {
  auto f = [](T x, T y) { return x + y; };
  return f(a, b);
}

template <typename T> T viaGenericLambda(T a, T b) {
  auto f = [](auto x, auto y) { return x + y; };
  return f(a, b);
}

int main(int argc, char **) {
  {
    Box<int> box(argc, 2);
    if (box.value != 3 || box.add(4) != 7) {
      return 1;
    }
  }
  return destroyed != 1 || viaLambda(argc, 2) != 3 || viaGenericLambda(argc, 2) != 3;
}

// Mutants are found in the instantiation the code is emitted for, the overload hides them in the templates

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp %libcxx -o %S/Output/O0.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/O0.exe | %filecheck %s --dump-input=fail --match-full-lines
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O2 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp %libcxx -o %S/Output/O2.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/O2.exe | %filecheck %s --dump-input=fail --match-full-lines
// CHECK:[info] Killed mutants (5/5):
// CHECK-NEXT:{{.*}}main.cpp:10:15: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:13:27: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:16:18: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:21:36: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:26:42: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:[info] Mutation score: 100%
