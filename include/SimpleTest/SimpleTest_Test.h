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
  std::vector<llvm::Function *> entryPoints() override {
    std::vector<llvm::Function *> entry;
    entry.push_back(TestFunction);
    return entry;
  }
  std::string getTestName() override;
  std::string getTestDisplayName() override;

  std::string getUniqueIdentifier() override { return getTestName(); };

  static bool classof(const Test *T) {
    return T->getKind() == TK_SimpleTest;
  }
};

}
