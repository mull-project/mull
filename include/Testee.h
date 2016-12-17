#pragma once

#include "llvm/IR/Function.h"

namespace Mutang {

class Testee {
  llvm::Function *testeeFunction;
  llvm::Function *callerFunction;
  int distance;

public:
  Testee(llvm::Function *testeeFunction,
         llvm::Function *callerFunction,
         int distance) : testeeFunction(testeeFunction),
                         callerFunction(callerFunction),
                         distance(distance) {}

  llvm::Function *getTesteeFunction() const {
    return testeeFunction;
  }

  llvm::Function *getCallerFunction() const {
    return callerFunction;
  }

  int getDistance() const {
    return distance;
  }
};

}
