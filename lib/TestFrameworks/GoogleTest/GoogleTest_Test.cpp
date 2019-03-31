#include "mull/TestFrameworks/GoogleTest/GoogleTest_Test.h"

using namespace mull;
using namespace std;

GoogleTest_Test::GoogleTest_Test(std::string Name, llvm::Function *TestBody,
                                 std::vector<llvm::Function *> Ctors)
    : Test(TK_GoogleTest), TestName(Name), TestBodyFunction(TestBody),
      GlobalCtors(Ctors) {}

std::string GoogleTest_Test::getTestName() { return TestName; }

std::string GoogleTest_Test::getTestDisplayName() { return getTestName(); }

std::vector<llvm::Function *> &GoogleTest_Test::GetGlobalCtors() {
  return GlobalCtors;
}

llvm::Function *GoogleTest_Test::GetTestBodyFunction() {
  return TestBodyFunction;
}
