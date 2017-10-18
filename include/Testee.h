#pragma once

#include <llvm/IR/Function.h>

namespace mull {

class Testee {
  llvm::Function *testeeFunction;
  int distance;

public:
  Testee(llvm::Function *testeeFunction,
         int distance) : testeeFunction(testeeFunction),
                         distance(distance) {}

  llvm::Function *getTesteeFunction() const {
    return testeeFunction;
  }

  int getDistance() const {
    return distance;
  }
};

}
