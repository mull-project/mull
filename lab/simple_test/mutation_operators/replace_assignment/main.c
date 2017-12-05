#include <stdio.h>

extern int test_replace_assignment();

int main() {
  int result = test_replace_assignment();

  printf("result of test_replace_assignment() is: %d\n", result);
}
