#include <stdio.h>

extern int test_math_div();

int main() {
  int result = test_math_div();

  printf("result of test_math_div() is: %d\n", result);
}
