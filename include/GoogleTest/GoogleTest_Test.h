#pragma once

#include "Test.h"

namespace llvm {

class Function;

}

namespace Mutang {

class GoogleTest_Test : public Test {
  llvm::Function *TestFunction;
public:
  GoogleTest_Test(llvm::Function *Function);
  llvm::Function *GetTestFunction();

  std::string getTestName() override;

  static bool classof(const Test *T) {
    return T->getKind() == TK_GoogleTest;
  }
};

}
