#include <stdio.h>
#include "test_and_or_operators.h"

int main() {
  if (test_OR_operator_2branches())
    return 1;
  if (test_OR_operator_1branch())
    return 1;
  if (test_OR_operator_always_scalars_case_with_function_call_pattern1())
    return 1;
  if (test_OR_operator_always_scalars_case_with_function_call_pattern3())
    return 1;
  return 0;
}
