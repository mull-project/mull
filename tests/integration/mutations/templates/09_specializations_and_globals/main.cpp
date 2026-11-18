// clang-format off
struct S {};
S operator+(S, S);

int base = 1;

template <typename T> T f(T x) {
  return x + 1;
}
template <> int f<int>(int x) {
  return x + 2;
}
template long f<long>(long);

template <typename T> struct Box {
  T value = 0;
  T add(T x);
  T get() {
    return value + 3;
  }
  friend T twice(Box b) {
    return b.value + b.value;
  }
  static T shared;
};

template <typename T> T Box<T>::add(T x) {
  return value + x;
}

template <> int Box<int>::get() {
  return value + 4;
}

template <typename T> T Box<T>::shared = T(base) + 5;

template <typename T> struct Box<T *> {
  T get(T *p) {
    return *p + 6;
  }
};

template <typename T> T scaled = T(base) + 7;

int global = base + 8;

int main() {
  Box<int> b;
  b.value = 2;
  Box<long> l;
  Box<int *> p;
  int one = 1;
  return f(1) != 3 || f(1L) != 2 || f(short(1)) != 2 || b.add(1) != 3 || b.get() != 6 ||
         l.get() != 3 || twice(b) != 4 || Box<int>::shared != 6 || p.get(&one) != 7 ||
         scaled<int> != 8 || global != 9;
}

// Explicit and partial specializations, explicit instantiations, out-of-line members, hidden friends,
// and globals initialized at startup: static members, variable templates and plain globals

// RUN: rm -rf %S/Output && mkdir -p %S/Output/sandbox
// RUN: cp %s %S/mull.yml %S/Output/sandbox/
// RUN: cd %S/Output/sandbox && %clang_cxx %sysroot -O0 -g -grecord-command-line %pass_mull_ir_frontend %S/Output/sandbox/main.cpp -o %S/Output/main.exe
// RUN: cd %S/Output/sandbox && %mull_runner -allow-surviving -reporters=IDE -ide-reporter-show-killed %S/Output/main.exe | %filecheck %s --dump-input=fail --match-full-lines --implicit-check-not=warning:
// CHECK:[info] Killed mutants (10/10):
// CHECK-NEXT:{{.*}}main.cpp:8:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:11:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:19:18: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:22:20: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:28:16: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:32:16: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:35:50: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:39:15: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:43:42: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:{{.*}}main.cpp:45:19: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:[info] Mutation score: 100%
