#pragma once

#include "Test.h"

namespace llvm {

class Function;

}

namespace mull {

class SimpleTest_Test : public Test {
  llvm::Function *TestFunction;
public:
  SimpleTest_Test(llvm::Function *Function);
  llvm::Function *GetTestFunction();

  std::string getTestName() override;

  std::string getUniqueIdentifier() override { return getTestName(); };

  static bool classof(const Test *T) {
    return T->getKind() == TK_SimpleTest;
  }
};

}
