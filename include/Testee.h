#pragma once

#include "llvm/IR/Function.h"

namespace Mutang {

class Testee {
  llvm::Function *testeeFunction;
  Testee *parentTestee;
  int distance;

public:
  Testee(llvm::Function *testeeFunction,
         Testee *parentTestee,
         int distance) : testeeFunction(testeeFunction),
                         parentTestee(parentTestee),
                         distance(distance) {}

  llvm::Function *getTesteeFunction() const {
    return testeeFunction;
  }

  Testee *getParentTestee() const {
    return parentTestee;
  }

  int getDistance() const {
    return distance;
  }
};

}
