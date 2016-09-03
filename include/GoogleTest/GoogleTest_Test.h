#pragma once

#include "Test.h"

#include <vector>

namespace llvm {

class Function;

}

namespace Mutang {

class GoogleTest_Test : public Test {
  std::string TestName;
  std::vector<llvm::Function *> GlobalCtors;
public:
  GoogleTest_Test(std::string Name, std::vector<llvm::Function *> Ctors);

  std::string getTestName() override;

  std::vector<llvm::Function *> &GetGlobalCtors();

  static bool classof(const Test *T) {
    return T->getKind() == TK_GoogleTest;
  }
};

}
