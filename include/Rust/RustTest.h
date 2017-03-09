#pragma once

#include "Test.h"

namespace llvm {

class Function;

}

namespace mull {

class RustTest : public Test {
  std::string testName;
  llvm::Function *function;
public:
  RustTest(std::string name, llvm::Function *function);

  std::string getTestName() override;
  std::string getTestDisplayName() override;

  std::string getUniqueIdentifier() override { return getTestName(); };

  llvm::Function *getFunction();

  static bool classof(const Test *T) {
    return T->getKind() == TK_RustTest;
  }
};

}
