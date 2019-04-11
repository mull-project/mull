#include <stdio.h>

enum { SUCCESS = 0, FAILURE = 1 };

int replace_assignment(int a) {
  int b = a + 100;

  // printf("a: %d, b: %d\n", a, b);

  return b;
}

int test_replace_assignment() {
  if (replace_assignment(1) == 101) {
    return SUCCESS;
  }
  return FAILURE;
}
