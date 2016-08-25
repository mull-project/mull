#pragma once

#include "Test.h"

#include <vector>

namespace llvm {

class Function;

}

namespace Mutang {

class GoogleTest_Test : public Test {
  llvm::Function *TestFunction;
  std::string TestName;
  std::string TestSuiteName;
  std::vector<llvm::Function *> GlobalCtors;
public:
  GoogleTest_Test(llvm::Function *Function, std::vector<llvm::Function *> Ctors);
  llvm::Function *GetTestFunction();

  std::string getTestName() override;
  std::string getTestSuiteName();

  std::vector<llvm::Function *> &GetGlobalCtors();

  static bool classof(const Test *T) {
    return T->getKind() == TK_GoogleTest;
  }

private:
  void DetermineSuiteAndTestNames();
};

}
