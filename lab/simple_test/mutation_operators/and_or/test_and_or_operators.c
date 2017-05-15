#include "test_and_or_operators.h"

#include <stdio.h>

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

int testee_OR_operator_2branches(int a, int b, int c) {
  if (a < b || b < c) {
    printf("left branch\n");
    return a;
  } else {
    printf("right branch\n");
    return b;
  }
}

int testee_OR_operator_1branch(int a, int b, int c) {
  if (a < b || b < c) {
    printf("left branch\n");
    return a;
  }

  printf("right branch\n");
  return b;
}

int testee_compound_AND_then_OR_operator(int a, int b, int c) {
  if (a < b && (b < c || a < c)) {
    printf("left branch\n");
    return a;
  }

  printf("right branch\n");
  return b;
}

int testee_compound_OR_then_AND_operator(int a, int b, int c) {
  if (a < b || (b < c && a < c)) {
    printf("left branch\n");
    return a;
  }

  printf("right branch\n");
  return b;
}

int test_AND_operator_2branches() {
  int result = (testee_AND_operator_2branches(1, 3, 2) == 3);
  return result;
}

int test_AND_operator_1branch() {
  int result = (testee_AND_operator_1branch(1, 3, 2) == 3);
  return result;
}

int test_OR_operator_2branches() {
  int result = (testee_OR_operator_2branches(1, 3, 2) == 1);
  return result;
}

int test_OR_operator_1branch() {
  int result = (testee_OR_operator_1branch(1, 3, 2) == 1);
  return result;
}

int test_compound_AND_then_OR_operator() {
  int result = (testee_compound_AND_then_OR_operator(1, 3, 2) == 1);
  return result;
}

int test_compound_OR_then_AND_operator() {
  int result = (testee_compound_OR_then_AND_operator(1, 3, 2) == 1);
  return result;
}
