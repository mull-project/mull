#include <stdio.h>
#include "test_and_or_operators.h"

int main() {
  printf("result of test_AND_operator_2branches() is: %d\n",
         test_AND_operator_2branches());

  printf("result of test_AND_operator_1branch() is: %d\n",
         test_AND_operator_1branch());

  printf("result of test_OR_operator_2branches() is: %d\n",
         test_OR_operator_2branches());

  printf("result of test_OR_operator_1branch() is: %d\n",
         test_OR_operator_1branch());

  printf("result of test_compound_AND_then_OR_operator() is: %d\n",
         test_compound_AND_then_OR_operator());

  printf("result of test_compound_OR_then_AND_operator() is: %d\n",
         test_compound_OR_then_AND_operator());
}
