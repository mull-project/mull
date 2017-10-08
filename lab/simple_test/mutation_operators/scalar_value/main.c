#include <stdio.h>

extern int test_scalar_value();

int main() {
  int result = test_scalar_value();

  printf("result of test_scalar_value() is: %d\n", result);
}
