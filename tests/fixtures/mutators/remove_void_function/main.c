#include <stdio.h>
#include "tester.h"

int main() {
  int result = test_func_with_a_void_function_inside();

  printf("result of test_func_with_a_void_function_inside() is: %d\n", result);

  return result;
}

