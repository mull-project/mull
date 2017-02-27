
#include "Testee.h"
#include "gtest/gtest.h"

TEST(HelloTest, testSumOfTestee) {
  Testee testeeClass;

  int result = testeeClass.sum(2, 3);

  EXPECT_EQ(result, 5);
}
