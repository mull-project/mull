#include "math_mul.h"

#include <stdio.h>

enum { SUCCESS = 0, FAILURE = 1 };

int math_mul(int a, int b) {
  return a * b;
}

int test_math_mul() {
  if (math_mul(6, 3) == 18) {
    return SUCCESS;
  }
  return FAILURE;
}
