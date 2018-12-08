#include <stdio.h>

int called_function(int x) {
  return x * x;
}

int replace_call(int a) {
  return a + called_function(a);
}

int test_replace_call() {
  int result = (replace_call(4) == 20);
  return result;
}
