template <typename T> T doSomething(T arg1, T arg2) {
  return arg1 + arg2;
}

int main() {
  if (doSomething(2, 3) != 5) {
    return 1;
  }
  if (doSomething(2.0, 0.0) != 2.0) {
    return 1;
  }
  return 0;
}

// clang-format off

// RUN: cd / && %clang_cxx %sysroot %s -fembed-bitcode -g -o %s.exe
// RUN: cd %CURRENT_DIR

// RUN: unset TERM; %mull_cxx -linker=%clang_cxx -linker-flags="%sysroot" -mutators=cxx_add_to_sub -ide-reporter-show-killed %s.exe 2>&1 | %filecheck %s --dump-input=fail --strict-whitespace --match-full-lines
// CHECK:[info] Killed mutants (1/1):
// CHECK:{{.*}}main.cpp:2:15: warning: Killed: Replaced + with - [cxx_add_to_sub]
// CHECK:  return arg1 + arg2;
// CHECK:              ^
// CHECK:[info] Mutation score: 100%
