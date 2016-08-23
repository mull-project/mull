#include "GoogleTest/GoogleTest_Test.h"

#include "llvm/IR/Function.h"

using namespace Mutang;

GoogleTest_Test::GoogleTest_Test(llvm::Function *Function) :
  Test(TK_SimpleTest), TestFunction(Function) {}

llvm::Function *GoogleTest_Test::GetTestFunction() {
  return TestFunction;
}

std::string GoogleTest_Test::getTestName() {
  return TestFunction->getName().str();
}
