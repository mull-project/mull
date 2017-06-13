#include <stdio.h>
#include "test_and_or_operators.h"

int main() {
  printf("result of test_OR_operator_with_CPP() is: %d\n",
         test_OR_operator_with_CPP());

  printf("result of test_OR_operator_with_CPP_PHI_case() is: %d\n",
         test_OR_operator_with_CPP_PHI_case());

  printf("result of test_OR_operator_with_CPP_and_assert() is: %d\n",
         test_OR_operator_with_CPP_and_assert());

  printf("result test_AND_operator_with_CPP() is: %d\n",
         test_AND_operator_with_CPP());

  printf("result of test_AND_operator_with_CPP_PHI_case() is: %d\n",
         test_AND_operator_with_CPP_PHI_case());

  printf("result of test_AND_operator_with_CPP_and_assert() is: %d\n",
         test_AND_operator_with_CPP_and_assert());
}
