#include "Laboratory.h"

#include "gtest/gtest.h"

using namespace Mutang;

TEST(HelloLab, GetLabName) {
  Laboratory lab;
  EXPECT_EQ(lab.name(), "Some Lab Name");
}

