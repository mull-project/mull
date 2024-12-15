#include <stdio.h>

extern int test_replace_call();

int main() {
  int result = test_replace_call();

  printf("result of test_replace_call() is: %d\n", result);
}
