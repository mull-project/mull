
#include "gtest/gtest.h"

class HelloWorld {

public:
  int sum(int a, int b);
};

int HelloWorld::sum(int a, int b) {
  int c = a + b;

  return c;
}

TEST(HelloWorld, sum) {
  HelloWorld hello;

  int result = hello.sum(2, 3);

  EXPECT_EQ(result, 5);
}
