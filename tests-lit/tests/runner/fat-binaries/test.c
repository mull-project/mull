int sum(int a, int b) {
  return a + b;
}

int main() {
  int r = sum(2, 5) == 7;
  return !r;
}

// clang-format off

/*

REQUIRES: MACOS
RUN: %clang_cc -arch arm64 -arch x86_64 %sysroot %s %pass_mull_ir_frontend -g -o %S/test-ir.exe

RUN: cd /; unset TERM; %mull_runner -ide-reporter-show-killed %S/test-ir.exe | %filecheck %s --dump-input=fail --match-full-lines --check-prefix=CHECK
CHECK:{{.*}}test.c:2:12: warning: Killed: Replaced + with - [cxx_add_to_sub]
CHECK:  return a + b;
CHECK:           ^
*/
