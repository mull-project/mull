
#include "gtest/gtest.h"

static int testee() {
  return 100;
}

TEST(HelloTest, testTesteeCalledAsInvokeInstr) {
  int res = -1;

  // try/catch is needed for invoke instruction instead of call instruction
  // to be generated for testee().
  try {
    res = testee();
  }
  catch (int e) {
    throw;
  }

  EXPECT_EQ(res, 100);
}
