#include <stdio.h>

int replace_assignment(int a) {
  int b = a + 100;

  // printf("a: %d, b: %d\n", a, b);

  return b;
}

int test_replace_assignment() {
  int result = (replace_assignment(1) == 101);
  return result;
}
