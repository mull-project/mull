#pragma once

#include "Test.h"

namespace llvm {

  class Function;

}

namespace mull {

  class XCTest_Test : public Test {
    std::string testName;
    llvm::Function *function;
  public:
    XCTest_Test(std::string name, llvm::Function *function);

    std::string getTestName() override;

    std::string getUniqueIdentifier() override { return getTestName(); };

    llvm::Function *getFunction();

    static bool classof(const Test *T) {
      return T->getKind() == TK_XCTest;
    }
  };
  
}
