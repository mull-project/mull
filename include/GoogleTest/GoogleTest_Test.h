#pragma once

#include "Test.h"

#include <vector>

namespace llvm {

class Function;

}

namespace mull {

class GoogleTest_Test : public Test {
  std::string TestName;
  llvm::Function *TestBodyFunction;
  std::vector<llvm::Function *> GlobalCtors;
public:
  GoogleTest_Test(std::string Name,
                  llvm::Function *TestBody,
                  std::vector<llvm::Function *> Ctors);

  std::string getTestName() override;
  std::string getTestDisplayName() override;

  std::string getUniqueIdentifier() override { return getTestName(); };

  std::vector<llvm::Function *> &GetGlobalCtors();
  llvm::Function *GetTestBodyFunction();

  std::vector<llvm::Function *> entryPoints() override {
    return std::vector<llvm::Function *>({ TestBodyFunction });
  }

  static bool classof(const Test *T) {
    return T->getKind() == TK_GoogleTest;
  }
};

}
