#include "tester.h"
#include "testee.h"

enum { SUCCESS = 0, FAILURE = 1 };

int test_func_with_a_void_function_inside() {
  if (testee()) {
    return SUCCESS;
  }
  return FAILURE;
}
