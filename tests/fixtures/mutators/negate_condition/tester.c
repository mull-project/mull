#include "tester.h"
#include "testee.h"

enum { SUCCESS = 0, FAILURE = 1 };

int test_max() {
  if (max(2, 4) == 4) {
    return SUCCESS;
  }
  return FAILURE;
}

