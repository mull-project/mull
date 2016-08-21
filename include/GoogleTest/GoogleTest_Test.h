#pragma once

#include "Test.h"

namespace llvm {

class Function;

}

namespace Mutang {

class GoogleTest_Test : public Test {
  llvm::Function *TestFunction;
  std::string TestName;
  std::string TestSuiteName;
public:
  GoogleTest_Test(llvm::Function *Function);
  llvm::Function *GetTestFunction();

  std::string getTestName() override;
  std::string getTestSuiteName();

  static bool classof(const Test *T) {
    return T->getKind() == TK_GoogleTest;
  }

private:
  void DetermineSuiteAndTestNames();
};

}
