
#include "Testee.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(HelloTest) {};

TEST(HelloTest, testSumOfTestee) {
  Testee testeeClass;

  int result = testeeClass.sum(2, 3);

  CHECK_EQUAL(result, 5);
}

TEST(HelloTest, testSumOfTestee2) {
  Testee testeeClass;

  int result = testeeClass.sum(2, 3);

  CHECK_EQUAL(result, 5);
}

