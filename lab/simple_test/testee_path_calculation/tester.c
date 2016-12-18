#include "tester.h"
#include "testee.h"
#include <stdio.h>

int test_testee_path_calculation() {
  int result = testee1();

  printf("test_testee_path_calculation returns result: %d\n", result > 1);

  return result > 1;
}

