#include "mull/TestFrameworks/SimpleTest/SimpleTest_Test.h"

#include <llvm/IR/Function.h>

using namespace mull;

SimpleTest_Test::SimpleTest_Test(llvm::Function *Function)
    : Test(TK_SimpleTest), TestFunction(Function) {}

llvm::Function *SimpleTest_Test::GetTestFunction() { return TestFunction; }

std::string SimpleTest_Test::getTestName() {
  return TestFunction->getName().str();
}

std::string SimpleTest_Test::getTestDisplayName() { return getTestName(); }
