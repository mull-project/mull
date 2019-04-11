#include "test_and_or_operators.h"

#include <assert.h>
#include <stdio.h>
#include <string>

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
  bool result = testee_OR_operator_with_CPP();
  return result == true;
}

int test_OR_operator_with_CPP_PHI_case() {
  bool result = testee_OR_operator_with_CPP_PHI_case();
  return result == true;
}

int test_OR_operator_with_CPP_and_assert() {
  bool result = testee_OR_operator_with_CPP_and_assert();
  return result == true;
}

int test_AND_operator_with_CPP() {
  bool result = testee_AND_operator_with_CPP();
  return result == false;
}

int test_AND_operator_with_CPP_PHI_case() {
  bool result = testee_AND_operator_with_CPP_PHI_case();
  return result == false;
}

int test_AND_operator_with_CPP_and_assert() {
  bool result = testee_AND_operator_with_CPP_and_assert();
  return result == true;
}

