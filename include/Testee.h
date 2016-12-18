#pragma once

#include "llvm/IR/Function.h"

namespace Mutang {

class Testee {
  llvm::Function *testeeFunction;
  Testee *callerTestee;
  int distance;

public:
  Testee(llvm::Function *testeeFunction,
         Testee *callerTestee,
         int distance) : testeeFunction(testeeFunction),
                         callerTestee(callerTestee),
                         distance(distance) {}

  llvm::Function *getTesteeFunction() const {
    return testeeFunction;
  }

  Testee *getCallerTestee() const {
    return callerTestee;
  }

  int getDistance() const {
    return distance;
  }
};

}
