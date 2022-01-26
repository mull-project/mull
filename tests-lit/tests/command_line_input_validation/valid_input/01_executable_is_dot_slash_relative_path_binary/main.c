int test(int a, int b) {
  return a + b;
}

int main(int argc, char **argv) {
  return !( test(2, 3) == 5 );
}

// clang-format off

// RUN: %clang_cc %pass_mull_ir_frontend  %s -g -o %s.exe
// RUN: %mull_runner main.c.exe -ide-reporter-show-killed | %filecheck %s --dump-input=fail --match-full-lines
// CHECK: [info] Killed mutants (1/1):
