#include "test_and_or_operators.h"

int main() {
  if (test_compound_AND_then_OR_operator())
    return 1;
  if (test_compound_AND_then_AND_operator())
    return 1;
  if (test_compound_OR_then_AND_operator())
    return 1;
  if (test_compound_OR_then_OR_operator())
    return 1;
  return 0;
}
