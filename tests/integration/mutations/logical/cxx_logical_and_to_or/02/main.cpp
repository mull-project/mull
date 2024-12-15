#include <assert.h>
#include <stdio.h>
#include <string>

enum { SUCCESS = 0, FAILURE = 1 };

bool testee_AND_operator_with_CPP() {
  std::string string1 = "STR1";
  std::string string2 = "STR2";

  if ((string1.find("STR1") != std::string::npos) && (string2.find("STR1") != std::string::npos)) {
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

  if ((string1.find("STR1") != std::string::npos) && (string2.find("STR1") != std::string::npos)) {
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

int main() {
  if (test_AND_operator_with_CPP())
    return 1;
  if (test_AND_operator_with_CPP_PHI_case())
    return 1;
  if (test_AND_operator_with_CPP_and_assert())
    return 1;
  return 0;
}

// clang-format off

// RUN: %clang_cxx %sysroot %TEST_CXX_FLAGS -O0 %pass_mull_ir_frontend -g %s -o %s-ir.exe
// RUN: unset TERM; %mull_runner -ide-reporter-show-killed -reporters=IDE %s-ir.exe | %filecheck %s --dump-input=fail
// CHECK:[info] Killed mutants (3/3):
// CHECK:{{.*}}main.cpp:11:51: warning: Killed: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if ((string1.find("STR1") != std::string::npos) &&
// CHECK:                                                  ^
// CHECK:{{.*}}main.cpp:26:51: warning: Killed: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  if ((string1.find("STR1") != std::string::npos) && (string2.find("STR1") != std::string::npos)) {
// CHECK:                                                  ^
// CHECK:{{.*}}main.cpp:41:3: warning: Killed: Replaced && with || [cxx_logical_and_to_or]
// CHECK:  assert(((string1.find("STR1") != std::string::npos) &&
// CHECK:  ^
// CHECK:[info] All mutations have been killed
// CHECK:[info] Mutation score: 100%
