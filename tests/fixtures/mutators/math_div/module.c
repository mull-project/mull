#include <stdio.h>

enum { SUCCESS = 0, FAILURE = 1 };

int math_div(int a, int b) {
  return a / b;
}

int test_math_div() {
  if (math_div(6, 3) == 2) {
    return SUCCESS;
  }
  return FAILURE;
}

int main() {
  return test_math_div();
}