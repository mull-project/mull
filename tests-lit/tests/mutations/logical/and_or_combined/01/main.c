extern int printf(const char *, ...);

enum { SUCCESS = 0, FAILURE = 1 };

int testee_compound_AND_then_OR_operator(int a, int b, int c) {
  if (a < b && (b < c || a < c)) {
    printf("left branch\n");
    return a;
  }

  printf("right branch\n");
  return b;
}

int testee_compound_AND_then_AND_operator(int A, int B, int C) {
  if ((!A && B) && C) {
    printf("left branch\n");
    return 1;
  } else {
    printf("right branch\n");
    return 0;
  }
}

int testee_compound_OR_then_AND_operator(int a, int b, int c) {
  // 1, 3, 2
  if (a < b || (b < c && a < c)) {
    printf("left branch\n");
    return a;
  }

  printf("right branch\n");
  return b;
}

int testee_compound_OR_then_OR_operator(int A, int B, int C) {
  if ((!A || B) || C) {
    printf("left branch\n");
    return 1;
  } else {
    printf("right branch\n");
    return 0;
  }
}

int test_compound_AND_then_OR_operator() {
  if (testee_compound_AND_then_OR_operator(1, 3, 2) == 1) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_compound_AND_then_AND_operator() {
  if (testee_compound_AND_then_AND_operator(1, 1, 1) == 0) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_compound_OR_then_AND_operator() {
  if (testee_compound_OR_then_AND_operator(1, 3, 2) == 1) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_compound_OR_then_OR_operator() {
  if (testee_compound_OR_then_OR_operator(0, 0, 0) == 1) {
    return SUCCESS;
  }
  return FAILURE;
}

int main() {
  if (test_compound_AND_then_OR_operator())
    return 1;
  if (test_compound_AND_then_AND_operator())
    return 1;
  if (test_compound_OR_then_AND_operator())
    return 1;
  if (test_compound_OR_then_OR_operator())
    return 1;
  return 0;
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (4/8):
// CHECK:{{.*}}main.c:6:23: warning: Killed: Replaced || with && [cxx_logical_or_to_and]
// CHECK:  if (a < b && (b < c || a < c)) {
// CHECK:                      ^
// CHECK:{{.*}}main.c:16:11: warning: Killed: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if ((!A && B) && C) {
// CHECK:          ^
// CHECK:{{.*}}main.c:27:13: warning: Killed: Replaced || with && [cxx_logical_or_to_and]
// CHECK:  if (a < b || (b < c && a < c)) {
// CHECK:            ^
// CHECK:{{.*}}main.c:37:11: warning: Killed: Replaced || with && [cxx_logical_or_to_and]
// CHECK:  if ((!A || B) || C) {
// CHECK:          ^
// CHECK:[info] Survived mutants (4/8):
// CHECK:{{.*}}main.c:6:13: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if (a < b && (b < c || a < c)) {
// CHECK:            ^
// CHECK:{{.*}}main.c:16:17: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if ((!A && B) && C) {
// CHECK:                ^
// CHECK:{{.*}}main.c:27:23: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if (a < b || (b < c && a < c)) {
// CHECK:                      ^
// CHECK:{{.*}}main.c:37:17: warning: Survived: Replaced || with && [cxx_logical_or_to_and]
// CHECK:  if ((!A || B) || C) {
// CHECK:                ^
// CHECK:[info] Mutation score: 50%
