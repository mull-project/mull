#include "tester.h"
#include "testee.h"

int test_func_with_a_void_function_inside() {
  int result = testee();

  return result == 1;
}

