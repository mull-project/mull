#include "SimpleTest/SimpleTest_Test.h"

#include "llvm/IR/Function.h"

using namespace Mutang;

SimpleTest_Test::SimpleTest_Test(llvm::Function *Function) :
  Test(TK_SimpleTest), TestFunction(Function) {}

llvm::Function *SimpleTest_Test::GetTestFunction() {
  return TestFunction;
}

std::string SimpleTest_Test::getTestName() {
  return TestFunction->getName().str();
}
