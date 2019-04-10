#include "mull/TestFrameworks/GoogleTest/GoogleTest_Test.h"

using namespace mull;
using namespace std;

GoogleTest_Test::GoogleTest_Test(std::string Name, llvm::Function *TestBody)
    : Test(TK_GoogleTest), TestName(Name), TestBodyFunction(TestBody) {}

std::string GoogleTest_Test::getTestName() { return TestName; }

std::string GoogleTest_Test::getTestDisplayName() { return getTestName(); }

llvm::Function *GoogleTest_Test::GetTestBodyFunction() {
  return TestBodyFunction;
}
