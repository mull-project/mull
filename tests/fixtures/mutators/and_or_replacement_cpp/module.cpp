#include "test_and_or_operators.h"

#include <assert.h>
#include <stdio.h>
#include <string>

enum { SUCCESS = 0, FAILURE = 1 };

bool testee_OR_operator_with_CPP() {
  std::string string1 = "STR1";
  std::string string2 = "STR2";

  if ((string1.find("STR1") != std::string::npos) ||
      (string2.find("STR1") != std::string::npos)) {
    printf("left branch\n");
    return true;
  } else {
    printf("right branch\n");
    return false;
  }
}

bool testee_OR_operator_with_CPP_PHI_case() {
  std::string string1 = "STR1";
  std::string string2 = "STR2";

  bool result = false;

  if ((string1.find("STR1") != std::string::npos) ||
      (string2.find("STR1") != std::string::npos)) {
    printf("left branch\n");
    result = true;
  } else {
    printf("right branch\n");
    result = false;
  }

  printf("result is: %d\n", result);

  return result;
}

bool testee_OR_operator_with_CPP_and_assert() {
  std::string string1 = "STR1";
  std::string string2 = "STR2";

  assert((string1.find("STR1") != std::string::npos) ||
         (string2.find("STR1") != std::string::npos));

  return true;
}

bool testee_AND_operator_with_CPP() {
  std::string string1 = "STR1";
  std::string string2 = "STR2";

  if ((string1.find("STR1") != std::string::npos) &&
      (string2.find("STR1") != std::string::npos)) {
    printf("left branch\n");
    return true;
  } else {
    printf("right branch\n");
    return false;
  }
}

bool testee_AND_operator_with_CPP_PHI_case() {
  std::string string1 = "STR1";
  std::string string2 = "STR2";

  bool result = false;

  if ((string1.find("STR1") != std::string::npos) &&
      (string2.find("STR1") != std::string::npos)) {
    printf("left branch\n");
    result = true;
  } else {
    printf("right branch\n");
    result = false;
  }

  return result;
}

bool testee_AND_operator_with_CPP_and_assert() {
  std::string string1 = "STR1";
  std::string string2 = "STR2";

  assert(((string1.find("STR1") != std::string::npos) &&
         (string2.find("STR1") != std::string::npos)) == false);

  return true;
}

int test_OR_operator_with_CPP() {
  if (testee_OR_operator_with_CPP()) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_OR_operator_with_CPP_PHI_case() {
  if (testee_OR_operator_with_CPP_PHI_case()) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_OR_operator_with_CPP_and_assert() {
  if (testee_OR_operator_with_CPP_and_assert()) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_AND_operator_with_CPP() {
  if (!testee_AND_operator_with_CPP()) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_AND_operator_with_CPP_PHI_case() {
  if (!testee_AND_operator_with_CPP_PHI_case()) {
    return SUCCESS;
  }
  return FAILURE;
}

int test_AND_operator_with_CPP_and_assert() {
  if (testee_AND_operator_with_CPP_and_assert()) {
    return SUCCESS;
  }
  return FAILURE;
}
