#include <stdio.h>
#include "test_and_or_operators.h"

int main() {
  printf("result of test_AND_operator_2branches() is: %d\n",
         test_AND_operator_2branches());

  printf("result of test_AND_operator_1branch() is: %d\n",
         test_AND_operator_1branch());
}
