extern int printf(const char *, ...);

enum { SUCCESS = 0, FAILURE = 1 };

void dummy() {}

int testee_AND_operator_2branches(int a, int b, int c) {
  if (a < b && b < c) {
    printf("left branch\n");
    return a;
  } else {
    printf("right branch\n");
    return b;
  }
}

int testee_AND_operator_1branch(int a, int b, int c) {
  if (a < b && b < c) {
    printf("left branch\n");
    return a;
  }

  printf("right branch\n");
  return b;
}

/// Edge case: AND expression that always evaluates to a scalar value but also
/// contains a dummy function call (presence of a dummy function makes the
/// Branch instruction to be generated).
/// This case is based on https://github.com/mull-project/mull/issues/501.
/// The code below is based on the code generated by the csmith:
/// a() { ((b(), 9) || 9, 0) || a; }
int testee_AND_operator_always_scalars_case_with_function_call_pattern1(int A) {
  if ((((dummy(), 0) && 1), 1) && A) {
    printf("left branch\n");
    return 1;
  } else {
    printf("right branch\n");
    return 0;
  }
}

int testee_AND_operator_always_scalars_case_with_function_call_pattern3(int A) {
  if (((dummy(), 9) && 9), A) {
    return 1;
  } else {
    printf("right branch\n");
    return 0;
  }
}

int test_AND_operator_2branches() {
  if (testee_AND_operator_2branches(1, 3, 2) == 3) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_AND_operator_1branch() {
  if (testee_AND_operator_1branch(1, 3, 2) == 3) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_AND_operator_always_scalars_case_with_function_call_pattern1() {
  if (testee_AND_operator_always_scalars_case_with_function_call_pattern1(1) == 1) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_AND_operator_always_scalars_case_with_function_call_pattern3() {
  if (testee_AND_operator_always_scalars_case_with_function_call_pattern3(1) == 1) {
    return SUCCESS;
  }
  return FAILURE;
}

int main() {
  if (test_AND_operator_2branches())
    return 1;
  if (test_AND_operator_1branch())
    return 1;
  if (test_AND_operator_always_scalars_case_with_function_call_pattern1())
    return 1;
  if (test_AND_operator_always_scalars_case_with_function_call_pattern3())
    return 1;
  return 0;
}

// clang-format off

// RUN: %clang_cc %sysroot -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner --allow-surviving -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (2/5):
// CHECK:{{.*}}main.c:8:13: warning: Killed: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if (a < b && b < c) {
// CHECK:            ^
// CHECK:{{.*}}main.c:18:13: warning: Killed: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if (a < b && b < c) {
// CHECK:            ^
// CHECK:[info] Survived mutants (3/5):
// CHECK:{{.*}}main.c:34:22: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if ((((dummy(), 0) && 1), 1) && A) {
// CHECK:                     ^
// CHECK:{{.*}}main.c:34:32: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if ((((dummy(), 0) && 1), 1) && A) {
// CHECK:                               ^
// CHECK:{{.*}}main.c:44:21: warning: Survived: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if (((dummy(), 9) && 9), A) {
// CHECK:                    ^
// CHECK:[info] Mutation score: 40%
