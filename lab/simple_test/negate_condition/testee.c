#include "testee.h"

#include <stdio.h>

int max(int a, int b) {
  if (a < b) {
    printf("a < b branch\n");
    return b;
  } else {
    printf("a >= b branch\n");
    return a;
  }
}

