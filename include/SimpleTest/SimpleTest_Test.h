#pragma once

#include "Test.h"

namespace llvm {

class Function;

}

namespace Mutang {

  class SimpleTest_Test : public Test {
    llvm::Function *TestFunction;
  public:
    SimpleTest_Test(llvm::Function *Function);
    llvm::Function *GetTestFunction();

    static bool classof(const Test *T) {
      return T->getKind() == TK_SimpleTest;
    }
  };

}
