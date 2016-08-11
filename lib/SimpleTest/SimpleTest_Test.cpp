#include "SimpleTest/SimpleTest_Test.h"

using namespace Mutang;

SimpleTest_Test::SimpleTest_Test(llvm::Function *Function) :
  Test(TK_SimpleTest), TestFunction(Function) {}

llvm::Function *SimpleTest_Test::GetTestFunction() {
  return TestFunction;
}
