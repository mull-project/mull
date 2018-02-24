#include "math_mul.h"

#include <stdio.h>

int math_mul(int a, int b) {
  return a * b;
}

int test_math_mul() {
  int result = (math_mul(6, 3) == 18);
  return result;
}
