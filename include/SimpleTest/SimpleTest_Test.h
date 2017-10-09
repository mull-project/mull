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
  std::string getTestDisplayName() override;

  std::string getUniqueIdentifier() override { return getTestName(); };

  std::vector<llvm::Function *> entryPoints() override {
    return std::vector<llvm::Function *>({ TestFunction });
  }

  static bool classof(const Test *T) {
    return T->getKind() == TK_SimpleTest;
  }
};

}
