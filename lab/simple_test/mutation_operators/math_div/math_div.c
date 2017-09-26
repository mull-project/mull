#include <stdio.h>

int math_div(int a, int b) {
  return a / b;
}

int test_math_div() {
  int result = (math_div(6, 3) == 2);
  return result;
}
