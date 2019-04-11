#include "math_sub.h"

#include <stdio.h>

enum { SUCCESS = 0, FAILURE = 1 };

int math_sub(int a, int b) {
  return a - b;
}

int test_math_sub() {
  if (math_sub(2, 4) == -2) {
    return SUCCESS;
  }
  return FAILURE;
}
