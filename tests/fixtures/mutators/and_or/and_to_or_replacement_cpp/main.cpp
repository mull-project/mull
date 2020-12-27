#include "test_and_or_operators.h"

int main() {
  if (test_AND_operator_with_CPP())
    return 1;
  if (test_AND_operator_with_CPP_PHI_case())
    return 1;
  if (test_AND_operator_with_CPP_and_assert())
    return 1;
  return 0;
}
