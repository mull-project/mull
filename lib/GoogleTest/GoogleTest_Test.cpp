#include "GoogleTest/GoogleTest_Test.h"

using namespace Mutang;
using namespace std;

GoogleTest_Test::GoogleTest_Test(std::string Name,
                                 llvm::Function *TestBody,
                                 std::vector<llvm::Function *> Ctors) :
  Test(TK_GoogleTest), TestName(Name), TestBodyFunction(TestBody), GlobalCtors(Ctors)
{
}

std::string GoogleTest_Test::getTestName() {
  return TestName;
}

std::vector<llvm::Function *> &GoogleTest_Test::GetGlobalCtors() {
  return GlobalCtors;
}

llvm::Function *GoogleTest_Test::GetTestBodyFunction() {
  return TestBodyFunction;
}
