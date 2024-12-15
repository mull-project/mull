int sum(int a, int b) {
  return a + b;
}

int main() {
  int r = sum(2, 5) == 7;
  return !r;
}

// clang-format off

/*

RUN: %clang_cc %sysroot %s %pass_mull_ir_frontend -g -o %S/test.exe

RUN: cd /; unset TERM; %mull_runner --mutation-score-threshold 49 %S/test.exe
*/
