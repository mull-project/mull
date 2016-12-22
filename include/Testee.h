#pragma once

#include "llvm/IR/Function.h"

namespace Mutang {

class Testee {
  llvm::Function *testeeFunction;
  llvm::Instruction *callerInstruction;
  Testee *callerTestee;
  int distance;

public:
  Testee(llvm::Function *testeeFunction,
         llvm::Instruction *callerInstruction,
         Testee *callerTestee,
         int distance) : testeeFunction(testeeFunction),
                         callerInstruction(callerInstruction),
                         callerTestee(callerTestee),
                         distance(distance) {}

  llvm::Function *getTesteeFunction() const {
    return testeeFunction;
  }

  llvm::Instruction *getCallerInstruction() const {
    return callerInstruction;
  }

  Testee *getCallerTestee() const {
    return callerTestee;
  }

  int getDistance() const {
    return distance;
  }
};

}
