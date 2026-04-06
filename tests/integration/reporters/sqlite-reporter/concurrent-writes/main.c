int math(int a, int b) {
  int r = 0;
  r += a + b;
  r -= a - b;
  r += a + b;
  r -= a - b;
  r += a + b;
  r -= a - b;
  r += a + b;
  r -= a - b;
  r += a + b;
  r -= a - b;
  return r;
}

int main() {
  return math(1, 2) != 20;
}

// clang-format off
/**
RUN: %clang_cc %sysroot -g -O0 %pass_mull_ir_frontend %s -o test.exe

RUN: bash run.sh %python3 hold_lock.py %mull_runner concurrent.sqlite 3 test.exe 5000

RUN: %sqlite3 concurrent.sqlite 'SELECT COUNT(*) FROM mutant' | %filecheck %s
CHECK: 10
*/
