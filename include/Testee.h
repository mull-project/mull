#pragma once

#include <llvm/IR/Function.h>

namespace mull {
  class Test;

class Testee {
  llvm::Function *testeeFunction;
  Test *test;
  int distance;

public:
  Testee(llvm::Function *testeeFunction, Test *test, int distance)
  : testeeFunction(testeeFunction), test(test), distance(distance) {}

  llvm::Function *getTesteeFunction() const {
    return testeeFunction;
  }

  Test *getTest() const {
    return test;
  }

  int getDistance() const {
    return distance;
  }
};

}
