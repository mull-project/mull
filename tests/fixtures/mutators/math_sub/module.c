#include "math_sub.h"

#include <stdio.h>

int math_sub(int a, int b) {
  return a - b;
}

int test_math_sub() {
  int result = (math_sub(2, 4) == -2);
  return result;
}
